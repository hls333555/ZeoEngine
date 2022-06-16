#pragma once

#include <IconsFontAwesome5.h>

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

#define LEVEL_EDITOR			"Level Editor"
#define PARTICLE_EDITOR			ICON_FA_FIRE "  Particle Editor"
#define MATERIAL_EDITOR			ICON_FA_CIRCLE "  Material Editor"
#define TEXTURE_EDITOR			ICON_FA_CHESS_BOARD "  Texture Editor"

#define LEVEL_EDITOR_VIEW		ICON_FA_PLAY_CIRCLE "  Level Editor View"
#define SCENE_OUTLINE			ICON_FA_SITEMAP "  Scene Outline"
#define ENTITY_INSPECTOR		ICON_FA_INFO_CIRCLE "  Entity Inspector"
#define OPEN_ASSET				ICON_FA_FOLDER "  Open Asset"
#define SAVE_ASSET				ICON_FA_FOLDER "  Save Asset"
#define CONTENT_BROWSER			ICON_FA_FOLDER "  Content Browser"
#define CONSOLE					ICON_FA_TERMINAL "  Console"
#define STATS					ICON_FA_CHART_PIE "  Stats"
#define FRAME_DEBUGGER			ICON_FA_BUG "  Frame Debugger"
#define PREFERENCES				ICON_FA_COGS "  Preferences"
#define ABOUT					ICON_FA_QUESTION_CIRCLE "  About"
#define PARTICLE_EDITOR_VIEW	ICON_FA_PLAY_CIRCLE "  Particle Editor View"
#define PARTICLE_INSPECTOR		ICON_FA_INFO_CIRCLE "  Particle Inspector"
#define MATERIAL_EDITOR_VIEW	ICON_FA_PLAY_CIRCLE "  Material Editor View"
#define MATERIAL_INSPECTOR		ICON_FA_INFO_CIRCLE "  Material Inspector"
#define TEXTURE_EDITOR_VIEW		ICON_FA_PLAY_CIRCLE "  Texture Editor View"
#define TEXTURE_INSPECTOR		ICON_FA_INFO_CIRCLE "  Texture Inspector"

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
