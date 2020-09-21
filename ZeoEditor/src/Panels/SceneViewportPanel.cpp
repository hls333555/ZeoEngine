#include "Panels/SceneViewportPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Input.h"

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
		// Camera movement control
		class SceneCameraController : public ScriptableEntity
		{
			friend SceneViewportPanel;

		public:
			explicit SceneCameraController(SceneViewportPanel* contextPanel)
				: m_ContextPanel(contextPanel) {}

			virtual void OnUpdate(DeltaTime dt) override
			{
				if (!m_ContextPanel || !m_ContextPanel->IsHovering()) return;

				auto& transform = GetComponent<TransformComponent>().Transform;
				auto& sceneCamera = GetComponent<CameraComponent>().Camera;

				// Move speed is set based on the zoom level (OrhographicSize)
				float cameraPanSpeed = sceneCamera.GetOrhographicSize() / 2.0f;

				// Pan camera view by holding middle mouse button
				// TODO: This does not go well with camera rotation
				if (Input::IsMouseButtonPressed(2))
				{
					auto [x, y] = Input::GetMousePosition();
					if (!m_IsbMiddleMouseButtonFirstPressed)
					{
						transform[3][0] -= (x - m_LastPressedMousePosition.x) * cameraPanSpeed * dt;
						transform[3][1] += (y - m_LastPressedMousePosition.y) * cameraPanSpeed * dt;
					}
					m_IsbMiddleMouseButtonFirstPressed = false;
					m_LastPressedMousePosition = { x, y };
				}

				if (Input::IsMouseButtonReleased(2))
				{
					m_IsbMiddleMouseButtonFirstPressed = true;
				}
			}

		private:
			SceneViewportPanel* m_ContextPanel;
			glm::vec2 m_LastPressedMousePosition{ 0.0f };
			bool m_IsbMiddleMouseButtonFirstPressed = true;
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
