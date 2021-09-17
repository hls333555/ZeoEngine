#include "Panels/InspectorPanels.h"

#include <IconsFontAwesome5.h>

#include "Editors/EditorBase.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionHelper.h"

namespace ZeoEngine {

	void InspectorPanel::OnAttach()
	{
		PanelBase::OnAttach();

		GetContextEditor()->m_PreSceneCreate.connect<&InspectorPanel::MarkComponentInspectorsDirty>(this);
	}

	void InspectorPanel::DrawAddComponentButton(Entity entity)
	{
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
		ImVec2 textSize = ImGui::CalcTextSize("Add Component");
		ImGui::Indent((contentRegionAvailable.x - textSize.x) * 0.5f);

		if (ImGui::Button(ICON_FA_PLUS_CIRCLE "  Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopupWithPadding("AddComponent"))
		{
			if (m_bIsCategorizedComponentsDirty)
			{
				m_CategorizedComponents.clear();

				// Iterate all registered components
				for (const auto compType : entt::resolve())
				{
					// Inherent components can never be added
					auto bIsInherentComp = DoesPropExist(PropertyType::Inherent, compType);
					if (bIsInherentComp) continue;

					auto category = GetPropValue<const char*>(PropertyType::Category, compType);
					std::string categoryName = category ? *category : "Default";
					// Categorize components
					m_CategorizedComponents[categoryName].push_back(compType.info().hash());
				}

				m_bIsCategorizedComponentsDirty = false;
			}

			for (const auto& [category, compIds] : m_CategorizedComponents)
			{
				if (ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
				{
					for (const auto compId : compIds)
					{
						if (ImGui::Selectable(GetComponentDisplayNameFull(compId)))
						{
							auto compInstance = entity.AddComponentById(compId);
							// Instance may be null as AddComponentById() failed
							if (compInstance)
							{
								MarkComponentInspectorsDirty();
							}
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::EndPopup();
		}
	}

	void EntityInspectorPanel::OnAttach()
	{
		m_bAllowAddingComponents = true;
	}

	void EntityInspectorPanel::ProcessRender()
	{
		Entity selectedEntity = GetContextEditor()->GetContextEntity();
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
			OnSelectedEntityChanged();
			return;
		}
		if (selectedEntity)
		{
			DrawComponents(selectedEntity);
		}
		m_LastSelectedEntity = selectedEntity;
	}

	void EntityInspectorPanel::OnSelectedEntityChanged()
	{
		MarkComponentInspectorsDirty();
	}

	void ParticleInspectorPanel::ProcessRender()
	{
		Entity contextEntity = GetContextEditor()->GetContextEntity();
		if (contextEntity)
		{
			DrawComponents<CoreComponent, TransformComponent>(contextEntity);
		}
	}

}
