#pragma once

#include "Panels/PanelBase.h"

namespace ZeoEngine {

	class StatsPanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;

	};

}
