#pragma once

#include "Panels/SceneViewportPanel.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class ParticleViewportPanel : public SceneViewportPanel
	{
	public:
		using SceneViewportPanel::SceneViewportPanel;

		virtual void OnAttach() override;

		virtual void Snapshot(const std::string& imageName, uint32_t imageWidth = 0) override;

	private:
		virtual void ProcessRender() override;

		virtual void RenderToolbar() override;

		void CreatePreviewParticle(bool bIsFromOpenScene = false);
		void CreateDefaultParticleSystem();

	private:
		Entity m_PreviewParticleEntity;

	};

}
