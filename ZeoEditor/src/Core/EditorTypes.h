#pragma once

#include <IconsFontAwesome5.h>

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

#define SCENE_EDITOR			"Scene Editor"
#define PARTICLE_EDITOR			ICON_FA_FIRE "  Particle Editor"

#define SCENE_VIEW				ICON_FA_PLAY_CIRCLE "  Scene View"
#define SCENE_OUTLINE			ICON_FA_SITEMAP "  Scene Outline"
#define ENTITY_INSPECTOR		ICON_FA_INFO_CIRCLE "  Entity Inspector"
#define CONTENT_BROWSER			ICON_FA_FOLDER "  Content Browser"
#define CONSOLE					ICON_FA_TERMINAL "  Console"
#define STATS					ICON_FA_CHART_PIE "  Stats"
#define PREFERENCES				ICON_FA_COGS "  Preferences"
#define ABOUT					ICON_FA_QUESTION_CIRCLE "  About"
#define PARTICLE_VIEW			ICON_FA_PLAY_CIRCLE "  Particle View"
#define PARTICLE_INSPECTOR		ICON_FA_INFO_CIRCLE "  Particle Inspector"

	struct DockspaceSpec
	{
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

	struct PanelSpec
	{
		ImGuiWindowFlags WindowFlags = 0;
		ImVec2 Padding = ImVec2(3.0f, 3.0f);
		ImVec2Data InitialSize{ { 800.0f, 600.0f } };
	};

}
