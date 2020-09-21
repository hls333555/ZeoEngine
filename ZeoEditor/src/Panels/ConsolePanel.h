#pragma once

#include "Panels/EditorPanel.h"

namespace ZeoEngine {

	class ConsolePanel : public EditorPanel
	{
	public:
		using EditorPanel::EditorPanel;

		virtual void OnImGuiRender() override;

	};

}
