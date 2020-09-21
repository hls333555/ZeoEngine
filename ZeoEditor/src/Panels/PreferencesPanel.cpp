#include "Panels/PreferencesPanel.h"

#include <imgui/imgui.h>

#include "Engine/Core/Application.h"

namespace ZeoEngine {

	void PreferencesPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		EditorPanel::OnImGuiRender();

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow))
		{
			// TODO: Write preferences to a config file
			ImGui::ShowStyleSelector("Editor style");

			// VSync
			{
				static bool bEnableVSync = true;
				ImGui::Checkbox("VSync", &bEnableVSync);
				auto& window = Application::Get().GetWindow();
				window.SetVSync(bEnableVSync);
			}
		}
		ImGui::End();
	}

}
