#include "Panels/DataInspectorPanel.h"

#include "Dockspaces/MainDockspace.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void DataInspectorPanel::DrawComponents(Entity entity)
	{
		// Push entity id for later use
		ImGui::PushID(static_cast<uint32_t>(entity));

		if (m_bIsPreprocessedTypesDirty)
		{
			GetScene()->m_Registry.visit(entity, [this](const auto componentId)
			{
				const auto type = entt::resolve_type(componentId);
				PreprocessType(type);
			});
			m_bIsPreprocessedTypesDirty = false;
		}

		for (const auto type : m_PreprocessedTypes)
		{
			// Push type id for later use
			ImGui::PushID(type.type_id());

			m_DataInspector.ProcessType(type, entity);

			ImGui::PopID();
		}
		m_DataInspector.MarkPreprocessedDatasClean();

		ImGui::PopID();
	}

	void DataInspectorPanel::MarkPreprocessedTypesDirty()
	{
		m_PreprocessedTypes.clear();
		m_bIsPreprocessedTypesDirty = true;
	}

	void DataInspectorPanel::MarkPreprocessedDatasDirty()
	{
		m_DataInspector.MarkPreprocessedDatasDirty();
	}

	void DataInspectorPanel::PreprocessType(entt::meta_type type)
	{
		// Reverse type display order
		m_PreprocessedTypes.push_front(type);
	}

	void EntityInspectorPanel::RenderPanel()
	{
		Entity selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
		if (selectedEntity != m_LastSelectedEntity && m_LastSelectedEntity)
		{
			// For last frame
			// We need to check validity as entity may have been destroyed just now
			if (m_LastSelectedEntity.IsValid())
			{
				// Sometimes, selected entity is changed when certain input box is still active, ImGui::IsItemDeactivatedAfterEdit() of that item will not get called,
				// so we have to draw last entity's components once again to ensure all caches are applied
				DrawComponents(m_LastSelectedEntity);
			}
			m_LastSelectedEntity = selectedEntity;

			MarkPreprocessedTypesDirty();
			MarkPreprocessedDatasDirty();
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
