#include "Inspectors/EntityInspector.h"

#include <IconsFontAwesome5.h>

#include "Engine/ImGui/MyImGui.h"
#include "Worlds/EditorPreviewWorldBase.h"
#include "Inspectors/ComponentInspector.h"
#include "Engine/Utils/ReflectionUtils.h"

namespace ZeoEngine {

	EntityInspector::EntityInspector(const Ref<EditorPreviewWorldBase>& world)
	{
		world->m_OnContextEntityChanged.connect<&EntityInspector::OnSelectedEntityChanged>(this);
		CategorizeComponents();
	}

	void EntityInspector::Draw(Entity entity)
	{
		// We need to check validity as entity may have been destroyed just now
		if (m_LastEntity.IsValid())
		{
			// Sometimes, selected entity is changed while certain input box is still active, ImGui::IsItemDeactivatedAfterEdit() of that item will not get called,
			// so we have to draw last entity's components once again to ensure all caches are applied
			// TODO: Edit(2022.9.10): After refactoring the whole editor, this is broken... and cannot be fixed right now,
			// if ImGui supports no-live-edit, we can refactor DataWidget till then
			// https://github.com/ocornut/imgui/issues/701
			DrawInternal(m_LastEntity);
			m_LastEntity = {};
			return;
		}

		if (!entity) return;

		if (m_bShouldRebuildComponentInspectors)
		{
			RebuildComponentInspectors(entity);
			m_bShouldRebuildComponentInspectors = false;
		}

		DrawInternal(entity);
	}

	void EntityInspector::DrawInternal(Entity entity)
	{
		// Push entity id
		ImGui::PushID(static_cast<U32>(entity));

		for (auto it = m_ComponentInspectors.begin(); it != m_ComponentInspectors.end();)
		{
			const auto& compInspector = *it;
			compInspector->Draw(entity);
			if (const U32 compId = compInspector->GetWillRemoveComponentId())
			{
				entity.RemoveComponentById(compId);
				it = m_ComponentInspectors.erase(it);
			}
			else
			{
				++it;
			}
		}

		ImGui::PopID();

		// The following part will not have entity id pushed into ImGui!
		ImGui::Separator();

		// Add component button
		DrawAddComponentButton(entity);
	}

	void EntityInspector::CategorizeComponents()
	{
		m_CategorizedComponents.clear();

		// Iterate all registered components
		for (const auto compType : entt::resolve())
		{
			// Inherent components can never be added
			auto bIsInherentComp = ReflectionUtils::DoesPropertyExist(Reflection::Inherent, compType);
			if (bIsInherentComp) continue;

			auto category = ReflectionUtils::GetPropertyValue<const char*>(Reflection::Category, compType);
			std::string categoryName = category ? *category : "Default";
			// Categorize components
			m_CategorizedComponents[categoryName].push_back(compType.info().hash());
		}
	}

	void EntityInspector::DrawAddComponentButton(Entity entity)
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
			for (const auto& [category, compIds] : m_CategorizedComponents)
			{
				if (ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
				{
					for (const auto compId : compIds)
					{
						if (ImGui::Selectable(ReflectionUtils::GetComponentDisplayNameFull(compId)))
						{
							const auto compInstance = entity.AddComponentById(compId);
							// Instance may be null as AddComponentById() failed
							if (compInstance)
							{
								RebuildComponentInspectors(entity);
							}
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::EndPopup();
		}
	}

	void EntityInspector::OnSelectedEntityChanged(Entity entity, Entity lastEntity)
	{
		m_LastEntity = lastEntity;
		m_bShouldRebuildComponentInspectors = true;
	}

	void EntityInspector::RebuildComponentInspectors(Entity entity)
	{
		m_ComponentInspectors.clear();

		// Process components on this entity
		for (const auto compId : entity.GetOrderedComponentIds())
		{
			// Skip if there is no data registered
			if (!ReflectionUtils::DoesTypeContainData(compId)) continue;

			m_ComponentInspectors.emplace_back(CreateScope<ComponentInspector>(compId));
		}
	}

}
