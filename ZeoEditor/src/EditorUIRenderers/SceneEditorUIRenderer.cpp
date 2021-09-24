#include "EditorUIRenderers/SceneEditorUIRenderer.h"

#include <imgui_internal.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Panels/SceneViewPanel.h"
#include "Panels/SceneOutlinePanel.h"
#include "Panels/InspectorPanels.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/AboutPanel.h"

namespace ZeoEngine {

	void SceneEditorUIRenderer::OnAttach()
	{
		EditorUIRendererBase::OnAttach();

		m_DockspaceSpec.WindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
		m_DockspaceSpec.WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		CreatePanel<SceneViewPanel>(SCENE_VIEW);
		CreatePanel<SceneOutlinePanel>(SCENE_OUTLINE);
		CreatePanel<EntityInspectorPanel>(ENTITY_INSPECTOR);
		CreatePanel<ContentBrowserPanel>(CONTENT_BROWSER);
		CreatePanel<ConsolePanel>(CONSOLE);

		CreateMenu("File")
			.MenuItem<MenuItem_NewAsset>(ICON_FA_FILE "  New Scene", "CTRL+N")
			.MenuItem<MenuItem_LoadAsset>(ICON_FA_FILE_IMPORT "  Load Scene", "CTRL+O")
			.MenuItem<MenuItem_SaveAsset>(ICON_FA_SAVE "  Save Scene", "CTRL+S")
			.MenuItem<MenuItem_SaveAssetAs>(ICON_FA_SAVE "  Save Scene As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y")
			.MenuItem<MenuItem_Copy>(ICON_FA_COPY"  Copy", "CTRL+C")
			.MenuItem<MenuItem_Paste>(ICON_FA_PASTE "  Paste", "CTRL+V")
			.MenuItem<MenuItem_Cut>(ICON_FA_CUT "  Cut", "CTRL+X");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel<SceneViewPanel>>(SCENE_VIEW)
			.MenuItem<MenuItem_TogglePanel<SceneOutlinePanel>>(SCENE_OUTLINE)
			.MenuItem<MenuItem_TogglePanel<EntityInspectorPanel>>(ENTITY_INSPECTOR)
			.MenuItem<MenuItem_TogglePanel<ContentBrowserPanel>>(CONTENT_BROWSER)
			.MenuItem<MenuItem_TogglePanel<ConsolePanel>>(CONSOLE)
			.MenuItem<MenuItem_TogglePanel<StatsPanel>>(STATS)
			.MenuItem<MenuItem_TogglePanel<PreferencesPanel>>(PREFERENCES)
			.MenuItem<MenuItem_Seperator>()
			.MenuItem<MenuItem_ResetLayout>(ICON_FA_WINDOW_RESTORE "  Reset layout");

		CreateMenu("Help")
			.MenuItem<MenuItem_TogglePanel<AboutPanel>>(ABOUT);
		
	}

	void SceneEditorUIRenderer::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.2f, nullptr, &dockLeft);
		ImGuiID dockRightDown;
		ImGuiID dockRightUp = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Up, 0.4f, nullptr, &dockRightDown);
		ImGuiID dockLeftUp;
		ImGuiID dockLeftDown = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.3f, nullptr, &dockLeftUp);
		ImGuiID dockLeftUpRight;
		ImGuiID dockLeftUpLeft = ImGui::DockBuilderSplitNode(dockLeftUp, ImGuiDir_Left, 0.2f, nullptr, &dockLeftUpRight);
		ImGuiID dockLeftDownRight;
		ImGuiID dockLeftDownLeft = ImGui::DockBuilderSplitNode(dockLeftDown, ImGuiDir_Left, 0.5f, nullptr, &dockLeftDownRight);

		ImGui::DockBuilderDockWindow(SCENE_VIEW, dockLeftUpRight);
		ImGui::DockBuilderDockWindow(SCENE_OUTLINE, dockRightUp);
		ImGui::DockBuilderDockWindow(ENTITY_INSPECTOR, dockRightDown);
		ImGui::DockBuilderDockWindow(CONTENT_BROWSER, dockLeftDownLeft);
		ImGui::DockBuilderDockWindow(CONSOLE, dockLeftDownRight);
	}

}
