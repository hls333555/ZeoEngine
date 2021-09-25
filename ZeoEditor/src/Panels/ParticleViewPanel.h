#pragma once

#include "Panels/ViewPanelBase.h"

namespace ZeoEngine {

	class ParticleViewPanel : public ViewPanelBase
	{
	public:
		using ViewPanelBase::ViewPanelBase;

	private:
		virtual void ProcessRender() override;

		virtual void RenderToolbar() override;

	};

}
