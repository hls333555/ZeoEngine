#pragma once

#include "Panels/PanelBase.h"

namespace ZeoEngine {

	class ConsolePanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

	private:
		virtual void ProcessRender() override;

	};

}
