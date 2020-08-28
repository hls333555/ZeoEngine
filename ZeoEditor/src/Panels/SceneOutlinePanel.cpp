#include "SceneOutlinePanel.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	SceneOutlinePanel::SceneOutlinePanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneOutlinePanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneOutlinePanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Outline");

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity{ entityID, m_Context.get() };
			DrawEntityNode(entity);
		});

		ImGui::End();
	}

	void SceneOutlinePanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_OpenOnArrow : 0) | ImGuiTreeNodeFlags_Selected;
		bool bOpened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
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
