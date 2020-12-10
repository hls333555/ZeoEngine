#include "Panels/GameViewportPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/EditorDockspace.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	void GameViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		m_StopTexture = Texture2D::Create("assets/textures/Stop.png");
		m_ToolbarTextures[0] = m_PlayTexture->GetTexture();
		m_ToolbarTextures[1] = m_PauseTexture->GetTexture();
	}

	void GameViewportPanel::OnEvent(Event& e)
	{
		SceneViewportPanel::OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(GameViewportPanel::OnKeyPressed));
	}

	bool GameViewportPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		if ((!IsPanelFocused() && !IsPanelHovered()) || e.GetRepeatCount() > 0) return false;

		switch (e.GetKeyCode())
		{
			case Key::W:
			{
				m_GizmoType = ImGuizmo::TRANSLATE;
				break;
			}
			case Key::E:
			{
				m_GizmoType = ImGuizmo::ROTATE;
				break;
			}
			case Key::R:
			{
				m_GizmoType = ImGuizmo::SCALE;
				break;
			}
			case Key::Space:
			{
				m_GizmoType = m_GizmoType + 1 > ImGuizmo::SCALE ? ImGuizmo::TRANSLATE : m_GizmoType + 1;
				break;
			}
			default:
				return false;
		}

		return true;
	}

	void GameViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();

		RenderGizmo();

		//ImGuiWindow* window = ImGui::GetCurrentWindow();

		// Begin drop operation from Class Browser
		// Note: BeginDragDropTarget() does not support window as target
		//if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->Rect(), ImGui::GetCurrentWindow()->ID))
		//{
		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragGameObjectClass"))
		//	{
		//		// We use active camera instead of editor camera here because placing objects during PIE is allowed for now
		//			// It should be changed back to editor camera if that behavior is disabled
		//		const glm::vec2 result = ProjectScreenToWorld2D(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), ImGui::GetCurrentWindow(), m_ActiveCamera);

		//		// Spawn dragged Game Object to the level at mouse position
		//		// Note: It sesems that rttr::argument does not support initializer_list conversion, so we should explicitly call constructor for glm::vec3 here
		//		rttr::variant createdVar = (*(rttr::type*)payload->Data).create({ glm::vec3{ result.x, result.y, 0.1f } });
		//		GameObject* spawnedGameObject = createdVar.get_value<GameObject*>();
		//		if (spawnedGameObject != m_SelectedGameObject)
		//		{
		//			OnGameObjectSelectionChanged(m_SelectedGameObject);
		//		}
		//		// Set it selected
		//		m_SelectedGameObject = spawnedGameObject;
		//		m_SelectedGameObject->m_bIsSelectedInEditor = true;
		//	}
		//	ImGui::EndDragDropTarget();
		//}

		//float padding = 5.0f;
		//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + padding, ImGui::GetCursorPos().y + padding));

		//OnGameViewImGuiRender();
	}

	void GameViewportPanel::RenderToolbar()
	{
		const float buttonSize = 32.0f;
		// Place buttons at window center
		ImGui::Indent(ImGui::GetContentRegionAvail().x * 0.5f - buttonSize - GImGui->Style.FramePadding.x * 3.0f/* Both two sides of button and SameLine() have spacing. */);
		// Toggle play / stop
		if (ImGui::ImageButton(m_ToolbarTextures[0], { buttonSize, buttonSize }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
		{
			ToggleStopTexture();
			//if (pieState == PIEState::None)
			//{
			//	StartPIE();
			//}
			//else
			//{
			//	StopPIE();
			//}
		}
		ImGui::SameLine();
		// Toggle pause / resume
		if (ImGui::ImageButton(m_ToolbarTextures[1], { buttonSize, buttonSize }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
		{
			ToggleResumeTexture();
			//if (pieState == PIEState::Running)
			//{
			//	PausePIE();
			//}
			//else if (pieState == PIEState::Paused)
			//{
			//	ResumePIE();
			//}
		}

	}

	void GameViewportPanel::ToggleStopTexture()
	{
		m_ToolbarTextures[0] = m_ToolbarTextures[0] == m_PlayTexture->GetTexture() ? m_StopTexture->GetTexture() : m_PlayTexture->GetTexture();
	}

	void GameViewportPanel::ToggleResumeTexture()
	{
		if (m_ToolbarTextures[0] == m_StopTexture->GetTexture())
		{
			m_ToolbarTextures[1] = m_ToolbarTextures[1] == m_PauseTexture->GetTexture() ? m_PlayTexture->GetTexture() : m_PauseTexture->GetTexture();
		}
	}

	void GameViewportPanel::RenderGizmo()
	{
		Entity selectedEntity = GetContext()->GetContextEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			const ImRect InnerRect = ImGui::GetCurrentWindow()->InnerRect;
			ImGuizmo::SetRect(InnerRect.Min.x, InnerRect.Min.y, InnerRect.GetSize().x, InnerRect.GetSize().y);

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Selected entity
			auto& transformComp = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 entityTransform = transformComp.GetTransform();

			// Snapping
			bool bSnap = Input::IsKeyPressed(Key::LeftControl);
			// Snap to 0.5m for translation/scale
			float snapValue = 0.5f;
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::ROTATE)
			{
				snapValue = 45.0f;
			}
			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL,
				glm::value_ptr(entityTransform),
				nullptr, bSnap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 outTranslation, outRotation, outScale;
				Math::DecomposeTransform(entityTransform, outTranslation, outRotation, outScale);

				// This delta rotation prevents gimbal lock situation
				glm::vec3 deltaRotation = outRotation - transformComp.Rotation;
				transformComp.Translation = outTranslation;
				transformComp.Rotation += deltaRotation;
				transformComp.Scale = outScale;
			}
		}
	}

}
