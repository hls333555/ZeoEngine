#include "Panels/DataInspectorPanel.h"

#include "Dockspaces/MainDockspace.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void DataInspectorPanel::DrawComponents(Entity entity, const std::vector<uint32_t>& ignoredTypeIds)
	{
		// Push entity id
		ImGui::PushID(static_cast<uint32_t>(entity));
		{
			bool bIsAnyTypeRemoved = false;
			// Process types on this entity
			for (const auto typeId : entity.GetAllComponents())
			{
				if (std::find(ignoredTypeIds.cbegin(), ignoredTypeIds.cend(), typeId) != ignoredTypeIds.cend()) continue;

				// Push type id
				ImGui::PushID(typeId);
				{
					auto type = entt::resolve_type(typeId);
					if (m_DataInspector.ProcessType(type, entity))
					{
						bIsAnyTypeRemoved = true;
					}
				}
				ImGui::PopID();
			}
			if (bIsAnyTypeRemoved)
			{
				MarkPreprocessedDatasDirty();
			}
			else
			{
				m_DataInspector.MarkPreprocessedDatasClean();
			}

			// The following part will not have entity Id pushed into ImGui!
			ImGui::PopID();

			if (m_bAllowAddingComponents)
			{
				// Add component button
				DrawAddComponentButton(entity);
			}

		}
	}

	void DataInspectorPanel::DrawAddComponentButton(Entity entity)
	{
		ImGui::Separator();

		ImGui::Columns(1);
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
		ImVec2 textSize = ImGui::CalcTextSize("Add Component");
		ImGui::Indent((contentRegionAvailable.x - textSize.x) * 0.5f);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (m_bIsCategorizedTypesDirty)
			{
				// Iterate all registered components
				for (auto type : entt::resolve())
				{
					// Inherent types can never be added
					auto bIsInherentType = DoesPropExist(PropertyType::InherentType, type);
					if (bIsInherentType) continue;

					auto category = GetPropValue<const char*>(PropertyType::Category, type);
					std::string categoryName = category ? *category : "Default";
					// Categorize types
					m_CategorizedTypes[categoryName].push_back(type.type_id());
				}
				m_bIsCategorizedTypesDirty = false;
			}

			for (const auto& [category, typeIds] : m_CategorizedTypes)
			{
				if (ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
				{
					for (const auto typeId : typeIds)
					{
						// We want to display "full name" here instead of "display name"
						auto typeName = GetPropValue<const char*>(PropertyType::Name, entt::resolve_type(typeId));
						if (ImGui::Selectable(*typeName))
						{
							entity.AddTypeById(typeId);
							MarkPreprocessedDatasDirty();
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::EndPopup();
		}
	}

	void DataInspectorPanel::MarkPreprocessedDatasDirty()
	{
		m_DataInspector.MarkPreprocessedDatasDirty();
	}

	void EntityInspectorPanel::OnAttach()
	{
		m_bAllowAddingComponents = true;
	}

	void EntityInspectorPanel::RenderPanel()
	{
		Entity selectedEntity = GetContext<MainDockspace>()->GetSeletedEntity();
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

			MarkPreprocessedDatasDirty();
			return;
		}
		if (selectedEntity)
		{
			DrawComponents(selectedEntity);
		}
		m_LastSelectedEntity = selectedEntity;
	}

	#define DEFAULT_IGNORED_TYPEIDS { entt::type_info<CoreComponent>::id(), entt::type_info<TransformComponent>::id() }

	void ParticleInspectorPanel::RenderPanel()
	{
		Entity contextEntity = GetContext()->GetContextEntity();
		if (contextEntity)
		{
			DrawComponents(contextEntity, DEFAULT_IGNORED_TYPEIDS);
		}
	}

}
