#include "Panels/PreferencesPanel.h"

#include <imgui.h>

#include "Engine/Core/Application.h"

namespace ZeoEngine {

	void PreferencesPanel::OnAttach()
	{
		PanelBase::OnAttach();

		m_PanelSpec.WindowFlags = ImGuiWindowFlags_NoCollapse;
		m_PanelSpec.Padding = ImGui::GetStyle().WindowPadding;
	}

	void PreferencesPanel::ProcessRender()
	{
		// TODO: Write preferences to a config file

		// VSync
		{
			static bool bEnableVSync = true;
			ImGui::Checkbox("VSync", &bEnableVSync);
			auto& window = Application::Get().GetWindow();
			window.SetVSync(bEnableVSync);
		}
	}

}
