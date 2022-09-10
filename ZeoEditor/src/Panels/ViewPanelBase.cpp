#include "Panels/ViewPanelBase.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Utils/PathUtils.h"
#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	void ViewPanelBase::UpdateWorld(const Ref<EditorPreviewWorldBase>& world)
	{
		if (!world || world.get() == m_EditorWorld) return;

		if (m_EditorWorld)
		{
			UnbindCameraComponentConstructionDelegate(m_EditorWorld->GetActiveScene());
			m_EditorWorld->m_OnActiveSceneChanged.disconnect<&ViewPanelBase::BindCameraComponentConstructionDelegate>(this);
			if (auto& sceneRenderer = m_EditorWorld->GetSceneRenderer())
			{
				m_OnViewportResize.disconnect<&SceneRenderer::OnViewportResize>(sceneRenderer);
			}
		}

		m_EditorWorld = world.get();
		m_FrameBuffer = nullptr;
		if (auto& sceneRenderer = world->GetSceneRenderer())
		{
			m_FrameBuffer = sceneRenderer->GetFrameBuffer().get();
			m_OnViewportResize.connect<&SceneRenderer::OnViewportResize>(sceneRenderer);
		}
		world->m_OnActiveSceneChanged.connect<&ViewPanelBase::BindCameraComponentConstructionDelegate>(this);
	}

	void ViewPanelBase::ProcessUpdate(DeltaTime dt)
	{
		if (!m_FrameBuffer) return;

		// This solution will render the 'old' sized framebuffer onto the 'new' sized ImGuiPanel and store the 'new' size in m_LastViewportSize
		// The next frame will first resize the framebuffer as m_LastViewportSize differs from framebuffer's width/height before updating and rendering
		// This results in never rendering an empty (black) framebuffer
		if (FrameBufferSpec spec = m_FrameBuffer->GetSpec();
			m_LastViewportSize.x > 0.0f && m_LastViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_LastViewportSize.x || spec.Height != m_LastViewportSize.y))
		{
			OnViewportResize(m_LastViewportSize);
		}

		m_EditorWorld->GetEditorCamera().SetViewportHovered(IsPanelHovered());
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

	#if EditorCameraDebug
		ImGui::Begin("Editor Camera Debug");
		ImGui::Checkbox("IsLastViewportHovered", &m_EditorCamera.m_bLastIsViewportHovered);
		ImGui::Checkbox("IsFirstPress", &m_EditorCamera.m_bIsFirstPress);
		ImGui::Text("Mode: %d", m_EditorCamera.m_CameraControlModes);
		ImGui::End();
	#endif
	}

	void ViewPanelBase::ProcessEvent(Event& e)
	{
		if (!IsPanelHovered()) return;

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(ViewPanelBase::OnMouseScroll));
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(ViewPanelBase::OnKeyPressed));
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

	void ViewPanelBase::OnViewportResize(const Vec2& size) const
	{
		// Broadcast changes
		m_OnViewportResizeDel.publish(static_cast<U32>(size.x), static_cast<U32>(size.y));

		// Resize editor camera
		m_EditorWorld->GetEditorCamera().SetViewportSize(size.x, size.y);

		UpdateViewportSizeOnSceneCameras();
	}

	void ViewPanelBase::BindCameraComponentConstructionDelegate(const Ref<Scene>& scene)
	{
		scene->m_Registry.on_construct<CameraComponent>().connect<&ViewPanelBase::UpdateViewportSizeOnSceneCameras>(this);
	}

	void ViewPanelBase::UnbindCameraComponentConstructionDelegate(const Ref<Scene>& scene)
	{
		scene->m_Registry.on_construct<CameraComponent>().disconnect<&ViewPanelBase::UpdateViewportSizeOnSceneCameras>(this);
	}

	void ViewPanelBase::UpdateViewportSizeOnSceneCameras() const
	{
		// Resize non-FixedAspectRatio cameras
		auto view = m_EditorWorld->GetActiveScene()->m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComp = view.get<CameraComponent>(entity);
			if (!cameraComp.bFixedAspectRatio)
			{
				cameraComp.Camera.SetViewportSize(m_LastViewportSize);
			}
		}
	}

}
