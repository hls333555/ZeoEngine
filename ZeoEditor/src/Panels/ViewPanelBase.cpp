#include "Panels/ViewPanelBase.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/KeyCodes.h"
#include "Editors/EditorBase.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Core/AssetRegistry.h"

namespace ZeoEngine {

	void ViewPanelBase::OnAttach()
	{
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		GetContextEditor()->SetEditorCamera(&m_EditorCamera);
		GetContextEditor()->m_PostSceneCreate.connect<&ViewPanelBase::PostSceneCreate>(this);
	}

	void ViewPanelBase::ProcessUpdate(DeltaTime dt)
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

		m_EditorCamera.OnUpdate(dt, IsPanelFocused());
	}

	void ViewPanelBase::ProcessRender()
	{
		const auto workRect = ImGui::GetWindowWorkRect();
		m_LastViewportSize = workRect.GetSize();
		SetViewportBounds(workRect.Min.x, workRect.Min.y, workRect.GetSize().x, workRect.GetSize().y);

		// TODO: BlockSceneEvents
		GetContextEditor()->BlockSceneEvents(!IsPanelFocused() && !IsPanelHovered());

		// Draw framebuffer texture
		ImGui::GetWindowDrawList()->AddImageRounded(
			GetContextEditor()->GetFrameBuffer()->GetColorAttachment(),
			// Upper left corner for the UVs to be applied at
			workRect.Min,
			// Lower right corner for the UVs to be applied at
			workRect.Max,
			// The UVs have to be flipped
			{ 0.0f, 1.0f }, { 1.0f, 0.0f },
			IM_COL32_WHITE,
			ImGui::IsWindowDocked() ? 0.0f : 8.0f, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight);

		RenderToolbar();
	}

	void ViewPanelBase::ProcessEvent(Event& e)
	{
		if (!IsPanelHovered()) return;

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(ViewPanelBase::OnMouseScroll));
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(ViewPanelBase::OnKeyPressed));
	}

	bool ViewPanelBase::OnMouseScroll(MouseScrolledEvent& e)
	{
		return m_EditorCamera.OnMouseScroll(e);
	}

	bool ViewPanelBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::F)
		{
			GetContextEditor()->FocusContextEntity();
		}
		return false;
	}

	std::string ViewPanelBase::GetPanelTitle() const
	{
		std::string assetName = GetContextEditor()->GetAsset()->GetName();
		if (assetName.empty())
		{
			assetName = "Untitled";
		}
		return assetName + "###" + GetPanelName();
	}

	void ViewPanelBase::Snapshot(const std::string& assetPath, uint32_t imageWidth, bool bOverwriteThumbnail)
	{
		std::string thumbnailPath = ThumbnailManager::Get().GetAssetThumbnailPath(assetPath, {});
		if (!bOverwriteThumbnail && PathUtils::DoesPathExist(thumbnailPath)) return;

		m_SnapshotSpec.AssetPath = assetPath;
		m_SnapshotSpec.ThumbnailPath = thumbnailPath;
		m_SnapshotSpec.ImageWidth = imageWidth;
		GetContextEditor()->GetFrameBuffer()->Snapshot(m_SnapshotSpec.ThumbnailPath, m_SnapshotSpec.ImageWidth);
		AssetRegistry::Get().GetPathSpec<AssetSpec>(m_SnapshotSpec.AssetPath)->UpdateThumbnail();
	}

	void ViewPanelBase::SetViewportBounds(float x, float y, float width, float height)
	{
		m_ViewportBounds[0].x = x;
		m_ViewportBounds[0].y = y;
		m_ViewportBounds[1].x = x + width;
		m_ViewportBounds[1].y = y + height;
	}

	glm::vec2 ViewPanelBase::GetViewportSize() const
	{
		return { m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y };
	}

	std::pair<float, float> ViewPanelBase::GetMouseViewportPosition()
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		return { mx, my };
	}

	void ViewPanelBase::OnViewportResize(const glm::vec2& size)
	{
		// Resize FrameBuffer
		GetContextEditor()->GetFrameBuffer()->Resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

		// Resize editor camera
		m_EditorCamera.SetViewportSize(size.x, size.y);

		UpdateViewportSizeOnSceneCameras();
	}

	void ViewPanelBase::PostSceneCreate()
	{
		// This binding must be called after scene creation!
		GetContextEditor()->GetScene()->m_Registry.on_construct<CameraComponent>().template connect<&ViewPanelBase::UpdateViewportSizeOnSceneCameras>(this);
	}

	void ViewPanelBase::UpdateViewportSizeOnSceneCameras()
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
