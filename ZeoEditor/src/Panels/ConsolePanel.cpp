#include "Panels/ConsolePanel.h"

#include "Engine/Core/EditorLog.h"

namespace ZeoEngine {

	void ConsolePanel::RenderPanel()
	{
		EditorLog::s_EditorLog.Draw();

		// TODO: Console command
	}

}
