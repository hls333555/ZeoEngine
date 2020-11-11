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

namespace ZeoEngine {

	void MainDockspace::OnAttach()
	{
		m_bIsMainDockspace = true;
		m_SerializeAssetType = AssetType::Scene;

		EditorDockspace::OnAttach();

		GameViewportPanel* gameViewportPanel = new GameViewportPanel(EditorWindowType::Game_View, this, true);
		m_OnSceneCreate.connect<&SceneViewportPanel::CreatePreviewCamera>(gameViewportPanel);
		SceneOutlinePanel* sceneOutlinePanel = new SceneOutlinePanel(EditorWindowType::Scene_Outline, this, true);
		EntityInspectorPanel* entityInspectorPanel = new EntityInspectorPanel(EditorWindowType::Entity_Inspector, this, true);
		ConsolePanel* consolePanel = new ConsolePanel(EditorWindowType::Console, true);

		ParticleEditorDockspace* particleEditorDockspace = new ParticleEditorDockspace(EditorWindowType::Particle_Editor, m_EditorContext);
		PushDockspace(particleEditorDockspace);

		StatsPanel* statsPanel = new StatsPanel(EditorWindowType::Stats, false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, { { 300, 300 } });
		PreferencesPanel* preferencesPanel = new PreferencesPanel(EditorWindowType::Preferences, false, ImGuiWindowFlags_NoCollapse);
		AboutPanel* aboutPanel = new AboutPanel(EditorWindowType::About, false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize, { { 300, 200 } });

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
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Game_View, std::string(), gameViewportPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Scene_Outline, std::string(), sceneOutlinePanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Entity_Inspector, std::string(), entityInspectorPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Console, std::string(), consolePanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Particle_Editor, std::string(), particleEditorDockspace->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Stats, std::string(), statsPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Preferences, std::string(), preferencesPanel->GetShowPtr()));
			
			windowMenu->PushMenuItem(new MenuItem_Seperator());
			windowMenu->PushMenuItem(new MenuItem_ResetLayout("Reset layout", std::string()));
			PushMenu(windowMenu);
		}

		{
			EditorMenu* helpMenu = new EditorMenu("Help");
			helpMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::About, std::string(), aboutPanel->GetShowPtr()));
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

	void MainDockspace::CreateNewScene()
	{
		EditorDockspace::CreateNewScene();

		// Clear selected entity
		m_ContextEntity = {};
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

		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorWindowType::Game_View).c_str(), dockLeftUpRight);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorWindowType::Scene_Outline).c_str(), dockRightUp);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorWindowType::Entity_Inspector).c_str(), dockRightDown);
		//ImGui::DockBuilderDockWindow(CLASS_BROWSER_NAME, dockLeftUpLeft);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorWindowType::Console).c_str(), dockLeftDown);
	}

}
