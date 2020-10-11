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
