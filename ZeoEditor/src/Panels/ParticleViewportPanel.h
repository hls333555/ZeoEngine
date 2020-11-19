#pragma once

#include "Panels/SceneViewportPanel.h"

#include "Engine/Renderer/Texture.h"

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

	private:
		Ref<Texture2D> m_PauseTexture, m_ResumeTexture, m_ResimulateTexture;
		void* m_ToolbarTextures[2];

		ParticleSystemPreviewComponent* m_ParticlePreviewComp;

	};

}
