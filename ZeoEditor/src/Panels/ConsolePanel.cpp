#include "Panels/ConsolePanel.h"

#include "Engine/ImGui/EditorLog.h"

namespace ZeoEngine {

	void ConsolePanel::OnAttach()
	{
		PanelBase::OnAttach();

		m_PanelSpec.Padding = ImGui::GetStyle().WindowPadding;
	}

	void ConsolePanel::ProcessRender()
	{
		EditorLog::s_EditorLog.Draw();

		// TODO: Console command
	}

}
