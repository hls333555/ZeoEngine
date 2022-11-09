#include "Panels/PreferencesPanel.h"

#include <imgui.h>

#include "Engine/Core/Application.h"

namespace ZeoEngine {

	void PreferencesPanel::OnAttach()
	{
		PanelBase::OnAttach();

		SetFlags(ImGuiWindowFlags_NoCollapse);
		SetPadding(ImGui::GetStyle().WindowPadding);
	}

	void PreferencesPanel::ProcessRender()
	{
		// TODO: Write preferences to a config file

		// VSync
		{
			auto& window = Application::Get().GetWindow();
			static bool bEnableVSync = window.IsVSync();
			ImGui::Checkbox("VSync", &bEnableVSync);
			window.SetVSync(bEnableVSync);
		}
	}

}
