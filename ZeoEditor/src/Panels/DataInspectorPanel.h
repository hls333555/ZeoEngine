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
		void DrawComponents(Entity entity, const std::set<uint32_t>& ignoredComponentIds = {});

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
