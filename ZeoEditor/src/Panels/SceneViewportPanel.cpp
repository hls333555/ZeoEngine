#include "Panels/SceneViewportPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Input.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/MouseCodes.h"

namespace ZeoEngine {

	void SceneViewportPanel::OnAttach()
	{
		CreatePreviewCamera();
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
	}

	void SceneViewportPanel::RenderPanel()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		m_LastViewportSize = static_cast<glm::vec2>(window->InnerRect.Max) - static_cast<glm::vec2>(window->InnerRect.Min);

		// Draw framebuffer texture
		ImGui::GetWindowDrawList()->AddImage(
			GetFrameBuffer()->GetColorAttachment(),
			// Upper left corner for the UVs to be applied at
			window->InnerRect.Min,
			// Lower right corner for the UVs to be applied at
			window->InnerRect.Max,
			// The UVs have to be flipped
			{ 0.0f, 1.0f }, { 1.0f, 0.0f });
	}

	void SceneViewportPanel::CreatePreviewCamera()
	{
		// Camera control
		// TODO: Add support for perspective camera
		class SceneCameraController : public ScriptableEntity
		{
		public:
			explicit SceneCameraController(SceneViewportPanel* contextPanel)
				: m_ContextPanel(contextPanel) {}

			virtual void OnUpdate(DeltaTime dt) override
			{
				auto* cameraComp = TryGetComponent<CameraComponent>();
				if (!cameraComp) return;

				if (m_bIsMiddleMouseButtonFirstPressed)
				{
					if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
					{
						m_bIsMiddleMouseButtonFirstPressed = false;
						// Only allow panning if first press is inside context panel
						m_bShouldUpdate = m_ContextPanel && m_ContextPanel->IsHovering();
					}
				}
				
				// Pan camera view by holding middle mouse button
				if (m_bShouldUpdate)
				{
					auto& translation = GetComponent<TransformComponent>().Translation;
					auto& sceneCamera = cameraComp->Camera;

					// Move speed is set based on the zoom level (OrthographicSize)
					float cameraPanSpeed = sceneCamera.GetOrthographicSize() / 4.0f;
					
					const auto position = Input::GetMousePosition();
					if (!m_bIsMiddleMouseButtonFirstPressedWhenHovered)
					{
						// TODO: This moves pretty slowly on high framerate
						translation.x -= (position.x - m_LastPressedMousePosition.x) * cameraPanSpeed * dt;
						translation.y += (position.y - m_LastPressedMousePosition.y) * cameraPanSpeed * dt;
					}
					m_bIsMiddleMouseButtonFirstPressedWhenHovered = false;
					m_LastPressedMousePosition = position;
				}

				if (Input::IsMouseButtonReleased(Mouse::ButtonMiddle))
				{
					m_bIsMiddleMouseButtonFirstPressed = true;
					m_bIsMiddleMouseButtonFirstPressedWhenHovered = true;
					m_bShouldUpdate = false;
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
					float size = sceneCamera.GetOrthographicSize();
					size -= e.GetYOffset() * 0.25f;
					size = std::max(size, 0.25f);
					sceneCamera.SetOrthographicSize(size);
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

		const std::string cameraName = GetPanelName() + std::string(" Camera");
		m_PreviewCamera = GetScene()->CreateEntity(cameraName.c_str(), true);
		GetScene()->m_Registry.on_construct<CameraComponent>().connect<&SceneViewportPanel::InitCameraViewportSize>(this);
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

	void SceneViewportPanel::InitCameraViewportSize(entt::registry& registry, entt::entity entity)
	{
		auto& cameraComp = registry.get<CameraComponent>(entity);
		cameraComp.Camera.SetViewportSize(m_LastViewportSize);
	}

}
