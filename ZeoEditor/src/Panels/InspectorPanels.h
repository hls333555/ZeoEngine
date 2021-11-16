#pragma once

#include "Panels/PanelBase.h"

#include "Engine/GameFramework/Entity.h"
#include "Reflection/ComponentInspector.h"

namespace ZeoEngine {

	class InspectorPanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

	protected:
		template<typename... IgnoredComponents>
		void DrawComponents(Entity entity)
		{
			// Push entity id
			ImGui::PushID(static_cast<uint32_t>(entity));
			{
				if (m_bIsComponentInspectorsDirty)
				{
					m_ComponentInspectors.clear();

					// Process components on this entity
					for (const auto compId : entity.GetOrderedComponentIds())
					{
						// NOTE: This pair of brackets inside if statement are required for template argument expansion!
						if ((ShouldIgnoreComponent<IgnoredComponents>(compId) || ...)) continue;

						m_ComponentInspectors.emplace_back(compId, entity);
					}

					m_bIsComponentInspectorsDirty = false;
				}

				for (auto it = m_ComponentInspectors.begin(); it != m_ComponentInspectors.end();)
				{
					auto compId = it->ProcessComponent();
					if (compId != -1)
					{
						entity.RemoveComponentById(compId);
						it = m_ComponentInspectors.erase(it);
					}
					else
					{
						++it;
					}
				}
			}
			ImGui::PopID();

			// The following part will not have entity id pushed into ImGui!
			if (m_bAllowAddingComponents)
			{
				ImGui::Separator();

				// Add component button
				DrawAddComponentButton(entity);
			}
		}

		void MarkComponentInspectorsDirty() { m_bIsComponentInspectorsDirty = true; }

	private:
		template<typename IComponent>
		bool ShouldIgnoreComponent(uint32_t compId)
		{
			return entt::type_hash<IComponent>::value() == compId;
		}

		void DrawAddComponentButton(Entity entity);

	protected:
		bool m_bAllowAddingComponents = false;

	private:
		std::vector<ComponentInspector> m_ComponentInspectors;
		bool m_bIsComponentInspectorsDirty = true;

		/** Map from category to list of component ids, used to draw categorized components in AddComponent popup */
		std::map<std::string, std::vector<uint32_t>> m_CategorizedComponents;
		bool m_bIsCategorizedComponentsDirty = true;

	};

	class EntityInspectorPanel : public InspectorPanel
	{
	public:
		using InspectorPanel::InspectorPanel;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;

		void OnSelectedEntityChanged();

	private:
		Entity m_LastSelectedEntity;

	};

	class ParticleInspectorPanel : public InspectorPanel
	{
	public:
		using InspectorPanel::InspectorPanel;

	private:
		virtual void ProcessRender() override;

	};

	class MaterialInspectorPanel : public InspectorPanel
	{
	public:
		using InspectorPanel::InspectorPanel;

	private:
		virtual void ProcessRender() override;

	};

}
