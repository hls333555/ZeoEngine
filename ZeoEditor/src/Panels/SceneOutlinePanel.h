#pragma once

#include "Panels/EditorPanel.h"

namespace ZeoEngine {

	class SceneOutlinePanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;

	private:
		virtual void RenderPanel() override;

		void DrawEntityNode(Entity entity);

	};

}
