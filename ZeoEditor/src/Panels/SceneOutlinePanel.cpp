#include "Panels/SceneOutlinePanel.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"

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
		}
		ImGui::End();
	}

	void SceneOutlinePanel::DrawEntityNode(Entity entity)
	{
		auto& tagComp = entity.GetComponent<TagComponent>();
		if (tagComp.bForInternalUse) return;

		ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_OpenOnArrow : 0) | ImGuiTreeNodeFlags_Selected;
		bool bOpened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComp.Tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		if (bOpened)
		{
			ImGui::TreePop();
		}
		
	}

}
