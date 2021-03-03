#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"
#include "Reflection/DataInspector.h"

namespace ZeoEngine {

	class DataInspectorPanel : public ScenePanel
	{
		friend class DataInspector;

	public:
		using ScenePanel::ScenePanel;

	protected:
		template<typename Component>
		bool ShouldIgnoreComponent(uint32_t compId)
		{
			return entt::type_hash<Component>::value() == compId;
		}

		template<typename... IgnoredComponents>
		void DrawComponents(Entity entity)
		{
			// Push entity id
			ImGui::PushID(static_cast<uint32_t>(entity));
			{
				// Process components on this entity
				for (const auto compId : entity.GetOrderedComponentIds())
				{
					// NOTE: This pair of brackets inside if statement are required for template argument expansion!
					if ((ShouldIgnoreComponent<IgnoredComponents>(compId) || ...)) continue;

					// Push component id
					ImGui::PushID(compId);
					{
						const auto compType = entt::resolve(compId);
						m_DataInspector.ProcessComponent(compType, entity);
					}
					ImGui::PopID();
				}
				m_DataInspector.OnDrawComponentsComplete();
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

	private:
		void DrawAddComponentButton(Entity entity);

	protected:
		bool m_bAllowAddingComponents = false;
		DataInspector m_DataInspector{ this };

	private:
		/** Map from category to list of component ids, used to draw categorized components in AddComponent popup */
		std::map<std::string, std::vector<uint32_t>> m_CategorizedComponents;
		bool m_bIsCategorizedComponentsDirty = true;

	};

	class EntityInspectorPanel : public DataInspectorPanel
	{
	public:
		using DataInspectorPanel::DataInspectorPanel;

		virtual void OnAttach() override;

	private:
		virtual void RenderPanel() override;

	private:
		Entity m_LastSelectedEntity;

	};

	class ParticleInspectorPanel : public DataInspectorPanel
	{
	public:
		using DataInspectorPanel::DataInspectorPanel;

	private:
		virtual void RenderPanel() override;

	};

}
