#include "Panels/ConsolePanel.h"

#include "Widgets/EditorConsole.h"

namespace ZeoEngine {

	void ConsolePanel::OnAttach()
	{
		PanelBase::OnAttach();

		SetPadding(ImGui::GetStyle().WindowPadding);
	}

	void ConsolePanel::ProcessRender()
	{
		EditorConsole::s_Instance.Draw();
	}

}
