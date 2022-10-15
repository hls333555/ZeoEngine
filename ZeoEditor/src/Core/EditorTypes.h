#pragma once

#include <IconsFontAwesome5.h>
#include <imgui.h>

namespace ZeoEngine {

#define LEVEL_VIEW				ICON_FA_PLAY_CIRCLE "  Level View"
#define LEVEL_OUTLINE			ICON_FA_SITEMAP "  Level Outline"
#define INSPECTOR				ICON_FA_INFO_CIRCLE "  Inspector"
#define DETAILS					ICON_FA_INFO_CIRCLE "  Details"
#define OPEN_ASSET				ICON_FA_FOLDER "  Open Asset"
#define SAVE_ASSET				ICON_FA_FOLDER "  Save Asset"
#define CONTENT_BROWSER			ICON_FA_FOLDER "  Content Browser"
#define CONSOLE					ICON_FA_TERMINAL "  Console"
#define STATS					ICON_FA_CHART_PIE "  Stats"
#define FRAME_DEBUGGER			ICON_FA_BUG "  Frame Debugger"
#define PREFERENCES				ICON_FA_COGS "  Preferences"
#define ABOUT					ICON_FA_QUESTION_CIRCLE "  About"
#define ASSET_VIEW				ICON_FA_PLAY_CIRCLE "  Asset View"

	struct PanelSpec
	{
		ImGuiWindowFlags WindowFlags = 0;
		bool bDisableClose = false;
		ImVec2 Padding{ 3.0f, 3.0f };
		ImVec2 InitialSize{ 800.0f, 600.0f };
		ImGuiCond InitialSizeCondition = ImGuiCond_FirstUseEver;
	};

}
