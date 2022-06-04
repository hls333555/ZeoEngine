#pragma once

#include "Panels/PanelBase.h"

namespace ZeoEngine {

	class FrameDebugPanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;
	};

}
