#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EntityInspectorPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;

		virtual void OnImGuiRender() override;

	private:
		void DrawComponents(Entity entity);

	};

}
