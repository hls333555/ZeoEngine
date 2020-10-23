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
				if (componentId == entt::type_info<IdComponent>::id()) return;

				const auto type = entt::resolve_type(componentId);
				PreprocessType(type);
			});
			m_bIsPreprocessedTypesDirty = false;
		}

		bool bIsAnyTypeRemoved = false;
		for (const auto type : m_PreprocessedTypes)
		{
			// Push type id for later use
			ImGui::PushID(type.type_id());

			if (m_DataInspector.ProcessType(type, entity))
			{
				bIsAnyTypeRemoved = true;
			}

			ImGui::PopID();
		}
		m_DataInspector.MarkPreprocessedDatasClean();
		if (bIsAnyTypeRemoved)
		{
			MarkPreprocessedTypesDirty();
		}

		// TODO: Particle Inspector should not display this button
		// Add component
		{
			ImGui::Separator();

			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			ImVec2 textSize = ImGui::CalcTextSize("Add Component");
			ImGui::Indent((contentRegionAvailable.x - textSize.x) / 2.0f);

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				// TODO: Sort types in alphabetical order
				// List all available components
				entt::resolve([this, entity](auto type)
				{
					auto typeName = GetPropValue<const char*>(PropertyType::Name, type);
					if (IsTypeEqual<TagComponent>(type) || IsTypeEqual<TransformComponent>(type)) return;

					if (ImGui::MenuItem(*typeName))
					{
						AddType(type, GetScene()->m_Registry, entity);
						MarkPreprocessedTypesDirty();
						ImGui::CloseCurrentPopup();
					}
				});

				ImGui::EndPopup();
			}
		}

		ImGui::PopID();
	}

	void DataInspectorPanel::MarkPreprocessedTypesDirty()
	{
		m_DataInspector.MarkPreprocessedDatasDirty();
		m_PreprocessedTypes.clear();
		m_bIsPreprocessedTypesDirty = true;
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
