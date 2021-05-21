#include "Utils/EditorUtils.h"

#include <IconsFontAwesome5.h>
#include <magic_enum.hpp>

#include "Engine/Core/Assert.h"

namespace ZeoEngine {

	const char* EditorUtils::GetEditorName(EditorType editorType)
	{
		switch (editorType)
		{
			case EditorType::MainEditor:		return "Main Editor";
			case EditorType::ParticleEditor:	return ICON_FA_FIRE "  Particle Editor";
		}

		const char* typeStr = magic_enum::enum_name(editorType).data();
		ZE_CORE_ASSERT("Failed to get name from dockspace type: {0}!", typeStr);
		return nullptr;
	}

	const char* EditorUtils::GetPanelName(PanelType panelType)
	{
		switch (panelType)
		{
			case PanelType::GameView:			return ICON_FA_PLAY_CIRCLE "  Game View";
			case PanelType::SceneOutline:		return ICON_FA_SITEMAP "  Scene Outline";
			case PanelType::EntityInspector:	return ICON_FA_INFO_CIRCLE "  Entity Inspector";
			case PanelType::ContentBrowser:		return ICON_FA_FOLDER "  Content Browser";
			case PanelType::Console:			return ICON_FA_TERMINAL "  Console";
			case PanelType::Stats:				return ICON_FA_CHART_PIE "  Stats";
			case PanelType::Preferences:		return ICON_FA_COGS "  Preferences";
			case PanelType::About:				return ICON_FA_QUESTION_CIRCLE "  About";
			case PanelType::ParticleView:		return ICON_FA_PLAY_CIRCLE "  Particle View";
			case PanelType::ParticleInspector:	return ICON_FA_INFO_CIRCLE "  Particle Inspector";
		}

		const char* typeStr = magic_enum::enum_name(panelType).data();
		ZE_CORE_ASSERT("Failed to get name from panel type: {0}!", typeStr);
		return nullptr;
	}

	const char* EditorUtils::GetAssetIcon(AssetType assetType)
	{
		switch (assetType)
		{
			case AssetType::Scene:				return ICON_FA_MOUNTAIN " ";
			case AssetType::ParticleTemplate:	return ICON_FA_FIRE_ALT " ";
			case AssetType::Texture2D:			return ICON_FA_IMAGE " ";
		}

		const char* typeStr = magic_enum::enum_name(assetType).data();
		ZE_CORE_ASSERT("Failed to get icon from asset type: {0}!", typeStr);
		return ICON_FA_FILE " ";
	}

}
