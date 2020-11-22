#include "Dockspaces/MainDockspace.h"

#include <imgui_internal.h>

#include "EditorLayer.h"
#include "Panels/GameViewportPanel.h"
#include "Panels/SceneOutlinePanel.h"
#include "Panels/DataInspectorPanel.h"
#include "Panels/ConsolePanel.h"
#include "Dockspaces/ParticleEditorDockspace.h"
#include "Panels/StatsPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/AboutPanel.h"
#include "Menus/EditorMenuItem.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	void MainDockspace::OnAttach()
	{
		EditorDockspace::OnAttach();

		GameViewportPanel* gameViewportPanel = new GameViewportPanel(EditorPanelType::Game_View, this, true);
		SceneOutlinePanel* sceneOutlinePanel = new SceneOutlinePanel(EditorPanelType::Scene_Outline, this, true);
		EntityInspectorPanel* entityInspectorPanel = new EntityInspectorPanel(EditorPanelType::Entity_Inspector, this, true);
		ConsolePanel* consolePanel = new ConsolePanel(EditorPanelType::Console, true);

		ParticleEditorDockspace* particleEditorDockspace = new ParticleEditorDockspace(EditorDockspaceType::Particle_Editor, m_EditorContext);
		PushDockspace(particleEditorDockspace);

		StatsPanel* statsPanel = new StatsPanel(EditorPanelType::Stats, false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, { { 300, 300 } });
		PreferencesPanel* preferencesPanel = new PreferencesPanel(EditorPanelType::Preferences, false, ImGuiWindowFlags_NoCollapse);
		AboutPanel* aboutPanel = new AboutPanel(EditorPanelType::About, false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize, { { 300, 200 } });

		{
			EditorMenu* fileMenu = new EditorMenu("File");
			fileMenu->PushMenuItem(new MenuItem_NewScene("New Scene", "CTRL+N"));
			fileMenu->PushMenuItem(new MenuItem_OpenScene("Open Scene", "CTRL+O"));
			fileMenu->PushMenuItem(new MenuItem_SaveScene("Save Scene", "CTRL+S"));
			fileMenu->PushMenuItem(new MenuItem_SaveSceneAs("Save Scene As", "CTRL+ALT+S"));
			PushMenu(fileMenu);
		}

		{
			EditorMenu* editMenu = new EditorMenu("Edit");
			editMenu->PushMenuItem(new MenuItem_Undo("Undo", "CTRL+Z"));
			editMenu->PushMenuItem(new MenuItem_Redo("Redo", "CTRL+Y"));
			editMenu->PushMenuItem(new MenuItem_Copy("Copy", "CTRL+C"));
			editMenu->PushMenuItem(new MenuItem_Paste("Paste", "CTRL+V"));
			editMenu->PushMenuItem(new MenuItem_Cut("Cut", "CTRL+X"));
			PushMenu(editMenu);
		}

		{
			EditorMenu* windowMenu = new EditorMenu("Window");
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Game_View), "", gameViewportPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Scene_Outline), "", sceneOutlinePanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Entity_Inspector), "", entityInspectorPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Console), "", consolePanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorDockspaceType::Particle_Editor), "", particleEditorDockspace->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Stats), "", statsPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Preferences), "", preferencesPanel->GetShowPtr()));
			
			windowMenu->PushMenuItem(new MenuItem_Seperator());
			windowMenu->PushMenuItem(new MenuItem_ResetLayout("Reset layout", ""));
			PushMenu(windowMenu);
		}

		{
			EditorMenu* helpMenu = new EditorMenu("Help");
			helpMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::About), "", aboutPanel->GetShowPtr()));
			PushMenu(helpMenu);
		}

		PushPanel(gameViewportPanel);
		PushPanel(sceneOutlinePanel);
		PushPanel(entityInspectorPanel);
		PushPanel(consolePanel);
		PushPanel(statsPanel);
		PushPanel(preferencesPanel);
		PushPanel(aboutPanel);

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
