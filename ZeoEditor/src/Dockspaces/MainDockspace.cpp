#include "Dockspaces/MainDockspace.h"

#include <imgui_internal.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItem.h"
#include "Engine/Core/Serializer.h"
#include "Panels/GameViewportPanel.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	void MainDockspace::OnAttach()
	{
		EditorDockspace::OnAttach();

		CreatePanel(EditorPanelType::Game_View);
		CreatePanel(EditorPanelType::Scene_Outline);
		CreatePanel(EditorPanelType::Entity_Inspector);
		CreatePanel(EditorPanelType::Console);

		CreateMenu("File")
			.MenuItem<MenuItem_NewScene>("New Scene", "CTRL+N")
			.MenuItem<MenuItem_OpenScene>("Open Scene", "CTRL+O")
			.MenuItem<MenuItem_SaveScene>("Save Scene", "CTRL+S")
			.MenuItem<MenuItem_SaveSceneAs>("Save Scene As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>("Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>("Redo", "CTRL+Y")
			.MenuItem<MenuItem_Copy>("Copy", "CTRL+C")
			.MenuItem<MenuItem_Paste>("Paste", "CTRL+V")
			.MenuItem<MenuItem_Cut>("Cut", "CTRL+X");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Game_View)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Scene_Outline)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Entity_Inspector)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Console)
			.MenuItem<MenuItem_ToggleEditor>(EditorDockspaceType::Particle_Editor)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Stats)
			.MenuItem<MenuItem_TogglePanel>(EditorPanelType::Preferences)
			.MenuItem<MenuItem_Seperator>()
			.MenuItem<MenuItem_ResetLayout>("Reset layout");

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

		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorPanelType::Game_View).c_str(), dockLeftUpRight);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorPanelType::Scene_Outline).c_str(), dockRightUp);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorPanelType::Entity_Inspector).c_str(), dockRightDown);
		//ImGui::DockBuilderDockWindow(CLASS_BROWSER_NAME, dockLeftUpLeft);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorPanelType::Console).c_str(), dockLeftDown);
	}

}
