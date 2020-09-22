#include "Panels/EntityInspectorPanel.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"

namespace ZeoEngine {

	void EntityInspectorPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		ScenePanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
		{
			Entity selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
			if (selectedEntity)
			{
				DrawComponents(selectedEntity);
			}
		}
		ImGui::End();
	}

	void EntityInspectorPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.GetComponent<TransformComponent>().Transform;

				ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

				ImGui::TreePop();
			}
		}
	}

}
