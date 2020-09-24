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

		void DrawComponents(Entity entity);

	};

}
