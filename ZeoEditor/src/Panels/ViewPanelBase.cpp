#include "Panels/ViewPanelBase.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Utils/PathUtils.h"
#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	void ViewPanelBase::UpdateWorld(EditorPreviewWorldBase* world)
	{
		const auto lastWorld = m_EditorWorld;
		if (!world || world == lastWorld) return;

		m_EditorWorld = world;
		OnWorldChanged(world, lastWorld);
	}

	void ViewPanelBase::ProcessUpdate(DeltaTime dt)
	{
		if (!m_FrameBuffer) return;

		// This solution will render the 'old' sized framebuffer onto the 'new' sized ImGuiPanel and store the 'new' size in m_LastViewportSize
		// The next frame will first resize the framebuffer as m_LastViewportSize differs from framebuffer's width/height before updating and rendering
		// This results in never rendering an empty (black) framebuffer
		if (const auto& spec = m_FrameBuffer->GetSpec();
			m_LastViewportSize.x > 0.0f && m_LastViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_LastViewportSize.x || spec.Height != m_LastViewportSize.y))
		{
			OnViewportResize(m_LastViewportSize);
		}

		// All camera manipulation will cause panel focusing, but when manipulation starts, IsPanelFocused() will always return false due to disabling ImGui mouse
		auto& editorCamera = m_EditorWorld->GetEditorCamera();
		editorCamera.SetEnableManipulation(!ImGuizmo::IsUsing() && IsPanelFocused() || editorCamera.IsManipulating());
	}

	void ViewPanelBase::ProcessRender()
	{
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		m_LastViewportSize = ImGui::GetContentRegionAvail();

		// TODO: BlockSceneEvents
		//GetContextEditor()->BlockSceneEvents(!IsPanelFocused() && !IsPanelHovered());

		// Draw framebuffer texture
		if (m_FrameBuffer)
		{
			ImGui::ImageRounded(m_FrameBuffer->GetColorAttachment()->GetTextureID(), m_LastViewportSize,
				ImGui::IsWindowDocked() ? 0.0f : 8.0f,
				{ 0, 1 }, { 1, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 },
				ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight);
		}
	}

	void ViewPanelBase::ProcessEvent(Event& e)
	{
		if (!IsPanelHovered()) return;

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return OnMouseScroll(e); });
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
	}

	bool ViewPanelBase::OnMouseScroll(MouseScrolledEvent& e) const
	{
		return m_EditorWorld->GetEditorCamera().OnMouseScroll(e);
	}

	bool ViewPanelBase::OnKeyPressed(KeyPressedEvent& e) const
	{
		if (e.GetKeyCode() == Key::F)
		{
			m_EditorWorld->FocusContextEntity();
		}
		return false;
	}

	void ViewPanelBase::OnWorldChanged(EditorPreviewWorldBase* world, EditorPreviewWorldBase* lastWorld)
	{
		if (lastWorld)
		{
			if (const auto* sceneRenderer = lastWorld->GetSceneRenderer())
			{
				m_OnViewportResize.disconnect<&SceneRenderer::OnViewportResize>(*sceneRenderer);
			}
		}

		m_FrameBuffer.reset();
		if (const auto* sceneRenderer = world->GetSceneRenderer())
		{
			m_FrameBuffer = sceneRenderer->GetFrameBuffer();
			m_OnViewportResize.connect<&SceneRenderer::OnViewportResize>(*sceneRenderer);
		}
	}

	std::string ViewPanelBase::GetPanelTitle() const
	{
		std::string assetName = AssetRegistry::Get().GetAssetMetadata(m_EditorWorld->GetAsset()->GetHandle())->PathName;
		ZE_CORE_ASSERT(!assetName.empty());
		return fmt::format("{}###{}", assetName, GetPanelName());
	}

	void ViewPanelBase::Snapshot(U32 imageWidth, bool bOverwriteThumbnail) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(m_EditorWorld->GetAsset()->GetHandle());
		const auto thumbnailPath = ThumbnailManager::Get().GetAssetThumbnailPath(metadata);
		if (!bOverwriteThumbnail && PathUtils::Exists(thumbnailPath)) return;

		m_FrameBuffer->Snapshot(thumbnailPath, imageWidth);
		metadata->UpdateThumbnail();
	}

	Vec2 ViewPanelBase::GetViewportSize() const
	{
		return { m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y };
	}

	std::pair<float, float> ViewPanelBase::GetMouseViewportPosition() const
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		return { mx, my };
	}

	void ViewPanelBase::OnCameraComponentAdded(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		UpdateViewportSizeOnSceneCamera(entity.GetComponent<CameraComponent>());
	}

	void ViewPanelBase::OnViewportResize(const Vec2& size) const
	{
		// Broadcast changes
		m_OnViewportResizeDel.publish(static_cast<U32>(size.x), static_cast<U32>(size.y));

		// Resize editor camera
		m_EditorWorld->GetEditorCamera().SetViewportSize(size.x, size.y);

		const auto cameraView = m_EditorWorld->GetActiveScene()->GetComponentView<CameraComponent>();
		for (const auto e : cameraView)
		{
			auto [cameraComp] = cameraView.get(e);
			UpdateViewportSizeOnSceneCamera(cameraComp);
		}
	}

	void ViewPanelBase::UpdateViewportSizeOnSceneCamera(CameraComponent& cameraComp) const
	{
		if (!cameraComp.bFixedAspectRatio)
		{
			cameraComp.Camera.SetViewportSize(m_LastViewportSize);
		}
	}

}
