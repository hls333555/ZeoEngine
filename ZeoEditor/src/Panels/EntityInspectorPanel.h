#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EntityInspectorPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;

	private:
		virtual void RenderPanel() override;

		void DrawInherentComponents(Entity entity);
		void DrawComponents(Entity entity);

		void ProcessIntegralData(entt::meta_data data);
		void ProcessEnumData(entt::meta_data data);
		void ProcessOtherData(entt::meta_data data);

	};

}
