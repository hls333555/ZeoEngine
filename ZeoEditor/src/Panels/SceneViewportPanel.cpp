#include "Panels/SceneViewportPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Input.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/MouseCodes.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	void SceneViewportPanel::OnAttach()
	{
		m_PlayTexture = Texture2D::Create("assets/textures/Play.png");
		m_PauseTexture = Texture2D::Create("assets/textures/Pause.png");

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		GetContext()->SetEditorCamera(&m_EditorCamera);
	}

	void SceneViewportPanel::OnUpdate(DeltaTime dt)
	{
		// This solution will render the 'old' sized framebuffer onto the 'new' sized ImGuiPanel and store the 'new' size in m_LastViewportSize
		// The next frame will first resize the framebuffer as m_LastViewportSize differs from framebuffer's width/height before updating and rendering
		// This results in never rendering an empty (black) framebuffer
		if (FrameBufferSpec spec = GetFrameBuffer()->GetSpec();
			m_LastViewportSize.x > 0.0f && m_LastViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_LastViewportSize.x || spec.Height != m_LastViewportSize.y))
		{
			OnViewportResize(m_LastViewportSize);
		}

		if (IsPanelFocused())
		{
			m_EditorCamera.OnUpdate(dt);
		}
	}

	void SceneViewportPanel::OnEvent(Event& e)
	{
		if ((!IsPanelFocused() && !IsPanelHovered())) return;

		m_EditorCamera.OnEvent(e);
	}

	void SceneViewportPanel::RenderPanel()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		m_LastViewportSize = static_cast<glm::vec2>(window->InnerRect.Max) - static_cast<glm::vec2>(window->InnerRect.Min);

		GetContext()->BlockEvents(!IsPanelFocused() && !IsPanelHovered());

		// Draw framebuffer texture
		ImGui::GetWindowDrawList()->AddImage(
			GetFrameBuffer()->GetColorAttachment(),
			// Upper left corner for the UVs to be applied at
			window->InnerRect.Min,
			// Lower right corner for the UVs to be applied at
			window->InnerRect.Max,
			// The UVs have to be flipped
			{ 0.0f, 1.0f }, { 1.0f, 0.0f });

		RenderToolbar();
	}

	void SceneViewportPanel::Snapshot(const std::string& imageName, uint32_t imageWidth)
	{
		GetFrameBuffer()->Snapshot(imageName, static_cast<uint32_t>(m_LastViewportSize.x), static_cast<uint32_t>(m_LastViewportSize.y), imageWidth);
	}

	void SceneViewportPanel::OnViewportResize(const glm::vec2& size)
	{
		// Resize FrameBuffer
		GetFrameBuffer()->Resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

		// Resize editor camera
		m_EditorCamera.SetViewportSize(size.x, size.y);

		// Resize non-FixedAspectRatio cameras
		auto view = GetScene()->m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComp = view.get<CameraComponent>(entity);
			if (!cameraComp.bFixedAspectRatio)
			{
				cameraComp.Camera.SetViewportSize(size);
			}
		}
	}

}
