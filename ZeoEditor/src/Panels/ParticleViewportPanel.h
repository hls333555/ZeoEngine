#pragma once

#include "Panels/SceneViewportPanel.h"

namespace ZeoEngine {

	class ParticleViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

	private:
		virtual void RenderPanel() override;

	};

}
