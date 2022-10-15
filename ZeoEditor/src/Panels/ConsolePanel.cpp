#include "Panels/ConsolePanel.h"

#include "Engine/ImGui/EditorConsole.h"

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
