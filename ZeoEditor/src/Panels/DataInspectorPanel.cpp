#include "Panels/DataInspectorPanel.h"

#include "Dockspaces/MainDockspace.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void DataInspectorPanel::DrawComponents(Entity entity)
	{
		// Push entity id
		ImGui::PushID(static_cast<uint32_t>(entity));
		{
			bool bIsAnyTypeRemoved = false;
			// Process types on this entity
			for (const auto typeId : entity.GetAllComponents())
			{
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

			// Add component button
			if (m_bAllowAddingComponents)
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
					// TODO: Sort types in alphabetical order
					// List all registered components
					for (auto type : entt::resolve())
					{
						// Inherent types can never be added
						auto bIsInherentType = DoesPropExist(PropertyType::InherentType, type);
						if (bIsInherentType) continue;

						// We want to display "full name" here instead of "display name"
						auto typeName = GetPropValue<const char*>(PropertyType::Name, type);
						if (ImGui::MenuItem(*typeName))
						{
							entity.AddType(type, GetScene()->m_Registry);
							MarkPreprocessedDatasDirty();
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::EndPopup();
				}
			}
		}
		ImGui::PopID();
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

	void ParticleInspectorPanel::RenderPanel()
	{
		Entity contextEntity = GetContext()->GetContextEntity();
		if (contextEntity)
		{
			DrawComponents(contextEntity);
		}
	}

}
