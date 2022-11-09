#pragma once

#include "Panels/ViewPanelBase.h"

namespace ZeoEngine {

	class ParticleEditorViewPanel : public ViewPanelBase
	{
	public:
		using ViewPanelBase::ViewPanelBase;

	private:
		virtual void ProcessRender() override;

		void RenderToolbar();

	};

}
