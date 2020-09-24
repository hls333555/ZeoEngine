#include "Panels/GameViewportPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/ext/matrix_transform.hpp>

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/EngineGlobals.h"

namespace ZeoEngine {

	void GameViewportPanel::OnAttach()
	{
		SceneViewportPanel::OnAttach();

		m_ToolbarTextures[0] = m_PlayTexture = Texture2D::Create("assets/textures/Play.png");
		m_ToolbarTextures[1] = m_PauseTexture = Texture2D::Create("assets/textures/Pause.png");
		m_StopTexture = Texture2D::Create("assets/textures/Stop.png");

		// TODO:
		Entity defaultTextureEntity = GetScene()->CreateEntity("Default Texture", true);
		defaultTextureEntity.AddComponent<SpriteRendererComponent>(GetTexture2DLibrary()->Get("../ZeoEditor/assets/textures/Checkerboard_Alpha.png"), glm::vec4(1.0f), glm::vec2{ 50.0f, 50.0f });
		auto& transComp = defaultTextureEntity.GetComponent<TransformComponent>();
		transComp.Transform = glm::scale(transComp.Transform, glm::vec3(100.0f));

		Entity shipEntity = GetScene()->CreateEntity("Player Ship");
		shipEntity.AddComponent<SpriteRendererComponent>(GetTexture2DLibrary()->Load("../Sandbox/assets/textures/Ship.png"));
	}

	void GameViewportPanel::RenderPanel()
	{
		SceneViewportPanel::RenderPanel();

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

		RenderToolbar();
	}

	void GameViewportPanel::RenderToolbar()
	{
		// Place buttons at window center
		ImGui::Indent(ImGui::GetWindowSize().x / 2.0f - 40.0f);
		// Toggle play / stop
		if (ImGui::ImageButton(m_ToolbarTextures[0]->GetTexture(), ImVec2(32.0f, 32.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
		{
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
		if (ImGui::ImageButton(m_ToolbarTextures[1]->GetTexture(), ImVec2(32.0f, 32.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
		{
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

}
