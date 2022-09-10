#pragma once

#include "Panels/ViewPanelBase.h"

namespace ZeoEngine {

	class AssetViewPanel : public ViewPanelBase
	{
	public:
		explicit AssetViewPanel(std::string panelName);

	private:
		virtual void ProcessRender() override;
	};
	
}
