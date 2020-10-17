#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class DataInspectorPanel : public ScenePanel
	{
		friend class DataInspector;

	public:
		using ScenePanel::ScenePanel;

	protected:
		void DrawComponents(Entity entity);
		
		void MarkPreprocessedTypesDirty();

	private:
		void PreprocessType(entt::meta_type type);

	private:
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
