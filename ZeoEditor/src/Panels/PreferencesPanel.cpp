#include "Panels/PreferencesPanel.h"

#include <imgui.h>

#include "Engine/Core/Application.h"

namespace ZeoEngine {

	void PreferencesPanel::RenderPanel()
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
