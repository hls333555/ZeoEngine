#include "Dockspaces/MainDockspace.h"

#include <imgui_internal.h>

#include "Engine/Core/Application.h"
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

#define GAME_VIEW_NAME "Game View"
#define SCENE_OUTLINE_NAME "Scene Outline"
#define ENTITY_INSPECTOR_NAME "Entity Inspector"
#define CLASS_BROWSER_NAME "Class Browser"
#define CONSOLE_NAME "Console"
#define PARTICLE_EDITOR_NAME "Particle Editor"
#define STATS_NAME "Stats"
#define PREFERENCES_NAME "Preferences"
#define ABOUT_NAME "About"

namespace ZeoEngine {

	void MainDockspace::OnAttach()
	{
		m_bIsMainDockspace = true;

		EditorDockspace::OnAttach();

		GameViewportPanel* gameViewportPanel = new GameViewportPanel(GAME_VIEW_NAME, this, true);
		SceneOutlinePanel* sceneOutlinePanel = new SceneOutlinePanel(SCENE_OUTLINE_NAME, this, true);
		EntityInspectorPanel* entityInspectorPanel = new EntityInspectorPanel(ENTITY_INSPECTOR_NAME, this, true);
		ConsolePanel* consolePanel = new ConsolePanel(CONSOLE_NAME, true);

		EditorLayer* editor = Application::Get().FindLayer<EditorLayer>();
		
		ParticleEditorDockspace* particleEditorDockspace = new ParticleEditorDockspace(PARTICLE_EDITOR_NAME);
		editor->PushDockspace(particleEditorDockspace);

		StatsPanel* statsPanel = new StatsPanel(STATS_NAME, false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, { { 300, 300 } });
		PreferencesPanel* preferencesPanel = new PreferencesPanel(PREFERENCES_NAME, false, ImGuiWindowFlags_NoCollapse);
		AboutPanel* aboutPanel = new AboutPanel(ABOUT_NAME, false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize, { { 300, 200 } });

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
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(GAME_VIEW_NAME, std::string(), gameViewportPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(SCENE_OUTLINE_NAME, std::string(), sceneOutlinePanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ENTITY_INSPECTOR_NAME, std::string(), entityInspectorPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(CONSOLE_NAME, std::string(), consolePanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(PARTICLE_EDITOR_NAME, std::string(), particleEditorDockspace->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(STATS_NAME, std::string(), statsPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(PREFERENCES_NAME, std::string(), preferencesPanel->GetShowPtr()));
			
			windowMenu->PushMenuItem(new MenuItem_Seperator());
			windowMenu->PushMenuItem(new MenuItem_ResetLayout("Reset layout", std::string()));
			PushMenu(windowMenu);
		}

		{
			EditorMenu* helpMenu = new EditorMenu("Help");
			helpMenu->PushMenuItem(new MenuItem_ToggleWindow(ABOUT_NAME, std::string(), aboutPanel->GetShowPtr()));
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

		ImGui::DockBuilderDockWindow(GAME_VIEW_NAME, dockLeftUpRight);
		ImGui::DockBuilderDockWindow(SCENE_OUTLINE_NAME, dockRightUp);
		ImGui::DockBuilderDockWindow(ENTITY_INSPECTOR_NAME, dockRightDown);
		ImGui::DockBuilderDockWindow(CLASS_BROWSER_NAME, dockLeftUpLeft);
		ImGui::DockBuilderDockWindow(CONSOLE_NAME, dockLeftDown);
	}

}
