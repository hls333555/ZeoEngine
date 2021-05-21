#include "Dockspaces/MainEditorDockspace.h"

#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	void MainEditorDockspace::OnAttach()
	{
		DockspaceBase::OnAttach();

		m_DockspaceSpec.WindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
		m_DockspaceSpec.WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		CreatePanel(PanelType::GameView);
		CreatePanel(PanelType::SceneOutline);
		CreatePanel(PanelType::EntityInspector);
		CreatePanel(PanelType::ContentBrowser);
		CreatePanel(PanelType::Console);

		CreateMenu("File")
			.MenuItem<MenuItem_NewScene>(ICON_FA_FILE "  New Scene", "CTRL+N")
			.MenuItem<MenuItem_OpenScene>(ICON_FA_FILE_IMPORT "  Open Scene", "CTRL+O")
			.MenuItem<MenuItem_SaveScene>(ICON_FA_SAVE "  Save Scene", "CTRL+S")
			.MenuItem<MenuItem_SaveSceneAs>(ICON_FA_SAVE "  Save Scene As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y")
			.MenuItem<MenuItem_Copy>(ICON_FA_COPY"  Copy", "CTRL+C")
			.MenuItem<MenuItem_Paste>(ICON_FA_PASTE "  Paste", "CTRL+V")
			.MenuItem<MenuItem_Cut>(ICON_FA_CUT "  Cut", "CTRL+X");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel>(PanelType::GameView)
			.MenuItem<MenuItem_TogglePanel>(PanelType::SceneOutline)
			.MenuItem<MenuItem_TogglePanel>(PanelType::EntityInspector)
			.MenuItem<MenuItem_TogglePanel>(PanelType::ContentBrowser)
			.MenuItem<MenuItem_TogglePanel>(PanelType::Console)
			.MenuItem<MenuItem_ToggleEditor>(EditorType::ParticleEditor)
			.MenuItem<MenuItem_TogglePanel>(PanelType::Stats)
			.MenuItem<MenuItem_TogglePanel>(PanelType::Preferences)
			.MenuItem<MenuItem_Seperator>()
			.MenuItem<MenuItem_ResetLayout>(ICON_FA_WINDOW_RESTORE "  Reset layout");

		CreateMenu("Help")
			.MenuItem<MenuItem_TogglePanel>(PanelType::About);
		
	}

	void MainEditorDockspace::PreRenderDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(mainViewport->WorkPos);
		ImGui::SetNextWindowSize(mainViewport->WorkSize);
		ImGui::SetNextWindowViewport(mainViewport->ID);
	}

	void MainEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
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
		ImGuiID dockLeftDownLeft = ImGui::DockBuilderSplitNode(dockLeftDown, ImGuiDir_Left, 0.3f, nullptr, &dockLeftDownRight);

		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::GameView), dockLeftUpRight);
		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::SceneOutline), dockRightUp);
		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::EntityInspector), dockRightDown);
		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::ContentBrowser), dockLeftDownLeft);
		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::Console), dockLeftDownRight);
	}

}
