#include "Panels/DataInspectorPanel.h"

#include "Dockspaces/MainDockspace.h"
#include "Reflection/DataInspector.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void DataInspectorPanel::DrawComponents(Entity entity)
	{
		// Push entity id for later use
		ImGui::PushID(static_cast<uint32_t>(entity));

		DataInspector dataInspector{ this };
		// We want to draw these components, if exist, first as the iteration order is backward
		if (entity.HasComponent<TagComponent>())
		{
			dataInspector.ProcessType(entt::resolve<TagComponent>(), entity);
		}
		if (entity.HasComponent<TransformComponent>())
		{
			dataInspector.ProcessType(entt::resolve<TransformComponent>(), entity);

			//if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			//{
			//	auto& transform = entity.GetComponent<TransformComponent>().Transform;

			//	ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

			//	ImGui::TreePop();
			//}
		}

		GetScene()->m_Registry.visit(entity, [this, &dataInspector, entity](const auto componentId)
		{
			const auto type = entt::resolve_type(componentId);
			if (IsTypeEqual<TagComponent>(type) || IsTypeEqual<TransformComponent>(type)) return;

			dataInspector.ProcessType(type, entity);
		});

		ImGui::PopID();
	}

	void EntityInspectorPanel::RenderPanel()
	{
		Entity selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
		if (selectedEntity != m_LastSelectedEntity && m_LastSelectedEntity)
		{
			// Sometimes, selected entity is changed when certain input box is still active, ImGui::IsItemDeactivatedAfterEdit() of that item will not get called,
			// so we have to draw last entity's components once again to ensure all caches are applied
			DrawComponents(m_LastSelectedEntity);
			m_LastSelectedEntity = selectedEntity;
			return;
		}
		if (selectedEntity)
		{
			DrawComponents(selectedEntity);
		}
		m_LastSelectedEntity = selectedEntity;
	}

	void ParticleInspectorPanel::RenderPanel()
	{
		Entity contextEntity = GetContext()->GetContextEntity();
		if (contextEntity)
		{
			DrawComponents(contextEntity);
		}
	}

}
