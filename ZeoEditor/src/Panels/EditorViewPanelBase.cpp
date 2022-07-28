#include "Panels/EditorViewPanelBase.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/KeyCodes.h"
#include "Editors/EditorBase.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	void EditorViewPanelBase::OnAttach()
	{
		m_EditorCamera = EditorCamera(50.625f, 1.778f, 0.1f, 1000.0f);
		GetContextEditor()->SetEditorCamera(&m_EditorCamera);
		GetContextEditor()->m_PostSceneCreate.connect<&EditorViewPanelBase::PostSceneCreate>(this);
	}

	void EditorViewPanelBase::ProcessUpdate(DeltaTime dt)
	{
		// This solution will render the 'old' sized framebuffer onto the 'new' sized ImGuiPanel and store the 'new' size in m_LastViewportSize
		// The next frame will first resize the framebuffer as m_LastViewportSize differs from framebuffer's width/height before updating and rendering
		// This results in never rendering an empty (black) framebuffer
		if (FrameBufferSpec spec = GetContextEditor()->GetFrameBuffer()->GetSpec();
			m_LastViewportSize.x > 0.0f && m_LastViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_LastViewportSize.x || spec.Height != m_LastViewportSize.y))
		{
			OnViewportResize(m_LastViewportSize);
		}

		m_EditorCamera.OnUpdate(dt, IsPanelHovered());
	}

	void EditorViewPanelBase::ProcessRender()
	{
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		m_LastViewportSize = ImGui::GetContentRegionAvail();

		// TODO: BlockSceneEvents
		GetContextEditor()->BlockSceneEvents(!IsPanelFocused() && !IsPanelHovered());

		// Draw framebuffer texture
		ImGui::ImageRounded(GetContextEditor()->GetFrameBuffer()->GetColorAttachment()->GetTextureID(), m_LastViewportSize,
			ImGui::IsWindowDocked() ? 0.0f : 8.0f,
			{ 0, 1 }, { 1, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 },
			ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight);

		// Drag asset to open
		if (ImGui::BeginDragDropTarget())
		{
			char typeStr[DRAG_DROP_PAYLOAD_TYPE_SIZE];
			_itoa_s(GetContextEditor()->GetAsset()->GetTypeID(), typeStr, 10);
			if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload(typeStr))
			{
				auto spec = *(const Ref<AssetMetadata>*)payload->Data;
				GetContextEditor()->LoadScene(spec->Path);
			}
			ImGui::EndDragDropTarget();
		}

		// Move cursor back to top
		ImGui::SetCursorPosY(viewportMinRegion.y);
		RenderToolbar();

	#if EditorCameraDebug
		ImGui::Begin("Editor Camera Debug");
		ImGui::Checkbox("IsLastViewportHovered", &m_EditorCamera.m_bLastIsViewportHovered);
		ImGui::Checkbox("IsFirstPress", &m_EditorCamera.m_bIsFirstPress);
		ImGui::Text("Mode: %d", m_EditorCamera.m_CameraControlModes);
		ImGui::End();
	#endif
	}

	void EditorViewPanelBase::ProcessEvent(Event& e)
	{
		if (!IsPanelHovered()) return;

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(EditorViewPanelBase::OnMouseScroll));
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(EditorViewPanelBase::OnKeyPressed));
	}

	bool EditorViewPanelBase::OnMouseScroll(MouseScrolledEvent& e)
	{
		return m_EditorCamera.OnMouseScroll(e);
	}

	bool EditorViewPanelBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::F)
		{
			GetContextEditor()->FocusContextEntity();
		}
		return false;
	}

	std::string EditorViewPanelBase::GetPanelTitle() const
	{
		auto assetName = AssetRegistry::Get().GetAssetMetadata(GetContextEditor()->GetAsset()->GetHandle())->PathName;
		if (assetName.empty())
		{
			assetName = "Untitled";
		}
		return assetName + "###" + GetPanelName();
	}

	void EditorViewPanelBase::Snapshot(const Ref<AssetMetadata>& metadata, U32 imageWidth, bool bOverwriteThumbnail) const
	{
		const auto thumbnailPath = ThumbnailManager::Get().GetAssetThumbnailPath(metadata);
		if (!bOverwriteThumbnail && PathUtils::Exists(thumbnailPath)) return;

		GetContextEditor()->GetFrameBuffer()->Snapshot(thumbnailPath.string(), imageWidth);
		metadata->UpdateThumbnail();
	}

	Vec2 EditorViewPanelBase::GetViewportSize() const
	{
		return { m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y };
	}

	std::pair<float, float> EditorViewPanelBase::GetMouseViewportPosition() const
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		return { mx, my };
	}

	void EditorViewPanelBase::OnViewportResize(const Vec2& size)
	{
		// Broadcast changes
		GetContextEditor()->m_OnViewportResizeDel.publish(static_cast<U32>(size.x), static_cast<U32>(size.y));

		// Resize editor camera
		m_EditorCamera.SetViewportSize(size.x, size.y);

		UpdateViewportSizeOnSceneCameras();
	}

	void EditorViewPanelBase::PostSceneCreate(const Ref<Scene>& scene)
	{
		// This binding must be called after scene creation!
		scene->m_Registry.on_construct<CameraComponent>().template connect<&EditorViewPanelBase::UpdateViewportSizeOnSceneCameras>(this);
	}

	void EditorViewPanelBase::UpdateViewportSizeOnSceneCameras()
	{
		// Resize non-FixedAspectRatio cameras
		auto view = GetContextEditor()->GetScene()->m_Registry.view<CameraComponent>();
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
