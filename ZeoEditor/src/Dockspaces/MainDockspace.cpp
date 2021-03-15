#include "Dockspaces/MainDockspace.h"

#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItem.h"
#include "Engine/Core/Serializer.h"
#include "Panels/GameViewportPanel.h"

namespace ZeoEngine {

	void MainDockspace::OnAttach()
	{
		EditorDockspace::OnAttach();

		CreatePanel(EditorPanelType::Game_View);
		CreatePanel(EditorPanelType::Scene_Outline);
		CreatePanel(EditorPanelType::Entity_Inspector);
		CreatePanel(EditorPanelType::Console);

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
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Game_View)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Scene_Outline)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Entity_Inspector)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Console)
			.MenuItem<MenuItem_ToggleEditor>(EditorDockspaceType::Particle_Editor)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Stats)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Preferences)
			.MenuItem<MenuItem_Seperator>()
			.MenuItem<MenuItem_ResetLayout>(ICON_FA_WINDOW_RESTORE "  Reset layout");

		CreateMenu("Help")
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::About);

	}

	void MainDockspace::CreateNewScene(bool bIsFromOpenScene)
	{
		EditorDockspace::CreateNewScene(bIsFromOpenScene);

		// Clear selected entity
		m_ContextEntity = {};
	}

	void MainDockspace::Serialize(const std::string& filePath)
	{
		SceneSerializer serializer(filePath, GetScene());
		serializer.Serialize();
	}

	void MainDockspace::Deserialize(const std::string& filePath)
	{
		SceneSerializer serializer(filePath, GetScene());
		serializer.Deserialize();
	}

	void MainDockspace::PostRenderScene(const Ref<FrameBuffer>& frameBuffer)
	{
		GameViewportPanel* gameViewportPanel = GetPanel<GameViewportPanel>(EditorPanelType::Game_View);
		gameViewportPanel->ReadPixelDataFromIDBuffer(frameBuffer);
	}

	int32_t MainDockspace::PreRenderDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowViewport(mainViewport->ID);
		ImGui::SetNextWindowPos(mainViewport->Pos);
		ImGui::SetNextWindowSize(mainViewport->Size);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_DockspaceSpec.Padding);
		return 3;
	}

	void MainDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.2f, nullptr, &dockLeft);
		ImGuiID dockRightDown;
		ImGuiID dockRightUp = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Up, 0.4f, nullptr, &dockRightDown);
		ImGuiID dockLeftUp;
		ImGuiID dockLeftDown = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.3f, nullptr, &dockLeftUp);
		ImGuiID dockLeftUpRight;
		ImGuiID dockLeftUpLeft = ImGui::DockBuilderSplitNode(dockLeftUp, ImGuiDir_Left, 0.2f, nullptr, &dockLeftUpRight);

		ImGui::DockBuilderDockWindow(GetPanelName(EditorPanelType::Game_View), dockLeftUpRight);
		ImGui::DockBuilderDockWindow(GetPanelName(EditorPanelType::Scene_Outline), dockRightUp);
		ImGui::DockBuilderDockWindow(GetPanelName(EditorPanelType::Entity_Inspector), dockRightDown);
		//ImGui::DockBuilderDockWindow(CLASS_BROWSER_NAME, dockLeftUpLeft);
		ImGui::DockBuilderDockWindow(GetPanelName(EditorPanelType::Console), dockLeftDown);
	}

}
