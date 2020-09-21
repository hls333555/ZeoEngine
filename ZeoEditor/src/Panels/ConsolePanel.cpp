#include "Panels/ConsolePanel.h"

#include <imgui/imgui.h>

#include "Engine/Core/EditorLog.h"

namespace ZeoEngine {

	void ConsolePanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		EditorPanel::OnImGuiRender();

		EditorLog::s_EditorLog.Draw(m_PanelName.c_str(), &m_bShow);
		// TODO: Console command
	}

}
