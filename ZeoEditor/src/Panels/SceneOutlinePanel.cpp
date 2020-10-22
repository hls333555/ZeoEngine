#include "Panels/SceneOutlinePanel.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	void SceneOutlinePanel::RenderPanel()
	{
		GetScene()->m_Registry.each([&](auto entityID)
		{
			Entity entity{ entityID, GetScene().get() };
			DrawEntityNode(entity);
		});

		// Deselect entity when blank space is clicked
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			GetContext<MainDockspace>()->m_SelectedEntity = {};
		}

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				GetScene()->CreateEntity("New Entity");
			}

			ImGui::EndPopup();
		}
	}

	void SceneOutlinePanel::DrawEntityNode(Entity entity)
	{
		auto& tagComp = entity.GetComponent<TagComponent>();
		if (tagComp.bIsInternal) return;

		auto& selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
		ImGuiTreeNodeFlags flags = (selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool bIsTreeExpanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComp.Name.c_str());
		if (ImGui::IsItemClicked())
		{
			selectedEntity = entity;
		}

		bool bIsCurrentEntitySelected = selectedEntity == entity;
		bool bWillDestroyEntity = false;
		if (bIsCurrentEntitySelected && ImGui::IsKeyReleased(Key::Delete))
		{
			bWillDestroyEntity = true;
		}

		if (bIsTreeExpanded)
		{
			// TODO: Display child entities

			ImGui::TreePop();
		}

		// Defer entity destruction
		if (bWillDestroyEntity)
		{
			GetScene()->DestroyEntity(entity);
			if (bIsCurrentEntitySelected)
			{
				selectedEntity = {};
			}
		}
		
	}

}
