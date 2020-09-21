#pragma once

#include "Panels/EditorPanel.h"

namespace ZeoEngine {

	class PreferencesPanel : public EditorPanel
	{
	public:
		using EditorPanel::EditorPanel;

		virtual void OnImGuiRender() override;

	};

}
