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
		void DrawComponents(Entity entity);

		void MarkPreprocessedDatasDirty();

	private:
		void DrawAddComponentButton(Entity entity);

	protected:
		bool m_bAllowAddingComponents{ false };
	private:
		DataInspector m_DataInspector{ this };

		/** Map from category to list of type ids, used to draw categorized types in AddComponent popup */
		std::map<std::string, std::vector<uint32_t>> m_CategorizedTypes;
		bool m_bIsCategorizedTypesDirty{ true };

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
