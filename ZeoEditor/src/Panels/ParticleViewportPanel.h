#pragma once

#include "Panels/SceneViewportPanel.h"

namespace ZeoEngine {

	struct ParticleSystemPreviewComponent;

	class ParticleViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;

		void CreatePreviewParticle(bool bIsFromOpenScene = false);

	private:
		virtual void RenderPanel() override;

		void CreateDefaultParticleSystem(ParticleSystemPreviewComponent& particleComp);

	};

}
