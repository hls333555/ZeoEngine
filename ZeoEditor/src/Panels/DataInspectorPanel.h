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
		
		void MarkPreprocessedTypesDirty();
		void MarkPreprocessedDatasDirty();

	private:
		void PreprocessType(entt::meta_type type);

	private:
		DataInspector m_DataInspector{ this };

		std::list<entt::meta_type> m_PreprocessedTypes;
		bool m_bIsPreprocessedTypesDirty{ true };

	};

	class EntityInspectorPanel : public DataInspectorPanel
	{
	public:
		using DataInspectorPanel::DataInspectorPanel;

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
