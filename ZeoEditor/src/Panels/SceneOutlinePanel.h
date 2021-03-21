#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class SceneOutlinePanel : public EditorPanel
	{
	public:
		using EditorPanel::EditorPanel;

	private:
		virtual void RenderPanel() override;

		void DrawEntityNode(Entity entity);

	};

}
