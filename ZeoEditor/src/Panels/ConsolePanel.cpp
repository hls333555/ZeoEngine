#include "Panels/ConsolePanel.h"

#include "Engine/ImGui/EditorLog.h"

namespace ZeoEngine {

	void ConsolePanel::RenderPanel()
	{
		EditorLog::s_EditorLog.Draw();

		// TODO: Console command
	}

}
