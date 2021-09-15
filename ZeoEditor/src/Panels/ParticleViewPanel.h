#pragma once

#include "Panels/ViewPanelBase.h"

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class ParticleViewPanel : public ViewPanelBase
	{
	public:
		using ViewPanelBase::ViewPanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;

		virtual void RenderToolbar() override;

		void CreatePreviewParticle(bool bIsFromOpen = false);
		void CreateDefaultParticleSystem();

	private:
		Entity m_PreviewParticleEntity;

	};

}
