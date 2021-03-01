#include "Panels/SceneOutlinePanel.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	void SceneOutlinePanel::RenderPanel()
	{
		// Display entities in creation order, the order is updated when a new entity is created
		GetScene()->m_Registry.view<CoreComponent>().each([this](auto entityId, auto& cc)
		{
			Entity entity{ entityId, GetScene().get() };
			DrawEntityNode(entity);
		});

		// Deselect entity when blank space is clicked
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			GetContext()->SetContextEntity({});
		}

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				auto selectedEntity = GetScene()->CreateEntity("New Entity");
				GetContext()->SetContextEntity(selectedEntity);
			}

			ImGui::EndPopup();
		}
	}

	void SceneOutlinePanel::DrawEntityNode(Entity entity)
	{
		auto& coreComp = entity.GetComponent<CoreComponent>();
		if (coreComp.bIsInternal) return;

		auto selectedEntity = GetContext()->GetContextEntity();
		ImGuiTreeNodeFlags flags = (selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool bIsTreeExpanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, coreComp.Name.c_str());
		if (ImGui::IsItemClicked())
		{
			GetContext()->SetContextEntity(entity);
		}

		bool bIsCurrentEntitySelected = selectedEntity == entity;
		bool bWillDestroyEntity = false;
		// TODO: Support viewport deletion
		if (IsPanelFocused() && bIsCurrentEntitySelected && ImGui::IsKeyReleased(Key::Delete))
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
				GetContext()->SetContextEntity({});
			}
		}
		
	}

}
