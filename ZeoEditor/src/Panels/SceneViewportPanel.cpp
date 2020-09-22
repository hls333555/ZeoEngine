#include "Panels/SceneViewportPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Input.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"

namespace ZeoEngine {

	void SceneViewportPanel::OnAttach()
	{
		CreatePreviewCamera();
	}

	void SceneViewportPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		ScenePanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			glm::vec2 max{ window->InnerRect.Max.x, window->InnerRect.Max.y };
			glm::vec2 min{ window->InnerRect.Min.x, window->InnerRect.Min.y };
			glm::vec2 size = max - min;
			if (size != m_LastViewportSize)
			{
				OnViewportResize(size);
				m_LastViewportSize = size;
			}

			// Draw framebuffer texture
			ImGui::GetWindowDrawList()->AddImage(
				GetFrameBuffer()->GetColorAttachment(),
				// Upper left corner for the UVs to be applied at
				window->InnerRect.Min,
				// Lower right corner for the UVs to be applied at
				window->InnerRect.Max,
				// The UVs have to be flipped
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		ImGui::End();
	}

	void SceneViewportPanel::CreatePreviewCamera()
	{
		// Camera control
		class SceneCameraController : public ScriptableEntity
		{
		public:
			explicit SceneCameraController(SceneViewportPanel* contextPanel)
				: m_ContextPanel(contextPanel) {}

			virtual void OnUpdate(DeltaTime dt) override
			{
				if (m_bIsMiddleMouseButtonFirstPressed)
				{
					if (Input::IsMouseButtonPressed(2))
					{
						m_bIsMiddleMouseButtonFirstPressed = false;
						// Only allow panning if first press is inside context panel
						m_bShouldUpdate = m_ContextPanel && m_ContextPanel->IsHovering();
					}
				}
				
				// Pan camera view by holding middle mouse button
				if (m_bShouldUpdate)
				{
					auto& transform = GetComponent<TransformComponent>().Transform;
					auto& sceneCamera = GetComponent<CameraComponent>().Camera;

					// Move speed is set based on the zoom level (OrthographicSize)
					float cameraPanSpeed = sceneCamera.GetOrhographicSize() / 4.0f;
					
					auto [x, y] = Input::GetMousePosition();
					if (!m_bIsMiddleMouseButtonFirstPressedWhenHovered)
					{
						transform[3][0] -= (x - m_LastPressedMousePosition.x) * cameraPanSpeed * dt;
						transform[3][1] += (y - m_LastPressedMousePosition.y) * cameraPanSpeed * dt;
					}
					m_bIsMiddleMouseButtonFirstPressedWhenHovered = false;
					m_LastPressedMousePosition = { x, y };
				}

				if (Input::IsMouseButtonReleased(2))
				{
					m_bIsMiddleMouseButtonFirstPressed = true;
					m_bIsMiddleMouseButtonFirstPressedWhenHovered = true;
				}
			}

			virtual void OnEvent(Event& e) override
			{
				EventDispatcher dispatcher(e);
				dispatcher.Dispatch<MouseScrolledEvent>(ZE_BIND_EVENT_FUNC(SceneCameraController::OnMouseScrolled));
				dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(SceneCameraController::OnKeyPressed));
			}

		private:
			bool OnMouseScrolled(MouseScrolledEvent& e)
			{
				if (m_ContextPanel && m_ContextPanel->IsHovering())
				{
					auto& sceneCamera = GetComponent<CameraComponent>().Camera;
					float size = sceneCamera.GetOrhographicSize();
					size -= e.GetYOffset() * 0.25f;
					size = std::max(size, 0.25f);
					sceneCamera.SetOrhographicSize(size);
				}
				
				return false;
			}

			bool OnKeyPressed(KeyPressedEvent& e)
			{
				return false;
			}

		private:
			SceneViewportPanel* m_ContextPanel;
			bool m_bShouldUpdate{ false };
			glm::vec2 m_LastPressedMousePosition{ 0.0f };
			bool m_bIsMiddleMouseButtonFirstPressed{ true }, m_bIsMiddleMouseButtonFirstPressedWhenHovered{ true };
		};

		const std::string cameraName = m_PanelName + std::string(" Camera");
		m_PreviewCamera = GetScene()->CreateEntity(cameraName.c_str(), true);
		m_PreviewCamera.AddComponent<CameraComponent>();
		NativeScriptComponent& nativeScriptComp = m_PreviewCamera.AddComponent<NativeScriptComponent>();
		nativeScriptComp.Bind<SceneCameraController>(this);
	}

	void SceneViewportPanel::OnViewportResize(const glm::vec2& size)
	{
		// Resize FrameBuffer
		GetFrameBuffer()->Resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

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
