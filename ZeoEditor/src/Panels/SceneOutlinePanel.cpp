#include "Panels/SceneOutlinePanel.h"

#include <imgui/imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"

namespace ZeoEngine {

	void SceneOutlinePanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		ScenePanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
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
		}
		ImGui::End();
	}

	void SceneOutlinePanel::DrawEntityNode(Entity entity)
	{
		auto& tagComp = entity.GetComponent<TagComponent>();
		if (tagComp.bIsInternal) return;

		auto& selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
		ImGuiTreeNodeFlags flags = (selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool bOpened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComp.Tag.c_str());
		if (ImGui::IsItemClicked())
		{
			selectedEntity = entity;
		}

		if (bOpened)
		{
			ImGui::TreePop();
		}
		
	}

}
