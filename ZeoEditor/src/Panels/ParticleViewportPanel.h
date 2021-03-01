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

		virtual void Snapshot(const std::string& imageName, uint32_t imageWidth = 0) override;

	private:
		virtual void RenderPanel() override;
		virtual void RenderToolbar() override;

		void CreateDefaultParticleSystem();

		void ToggleResumeTexture();

	private:
		Ref<Texture2D> m_ResimulateTexture;

		ParticleSystemPreviewComponent* m_ParticlePreviewComp;

	};

}