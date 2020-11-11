#include "Dockspaces/ParticleEditorDockspace.h"

#include <imgui_internal.h>

#include "Panels/ParticleViewportPanel.h"
#include "Panels/DataInspectorPanel.h"
#include "Menus/EditorMenuItem.h"

namespace ZeoEngine {

	void ParticleEditorDockspace::OnAttach()
	{
		m_SerializeAssetType = AssetType::ParticleTemplate;

		EditorDockspace::OnAttach();

		ParticleViewportPanel* particleViewportPanel = new ParticleViewportPanel(EditorWindowType::Particle_View, this, true);
		m_OnSceneCreate.connect<&ParticleViewportPanel::CreatePreviewParticle>(particleViewportPanel);
		m_OnSceneCreate.before<&ParticleViewportPanel::CreatePreviewParticle>(particleViewportPanel).connect<&SceneViewportPanel::CreatePreviewCamera>(particleViewportPanel);
		ParticleInspectorPanel* particleInspectorPanel = new ParticleInspectorPanel(EditorWindowType::Particle_Inspector, this, true);

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
			PushMenu(editMenu);
		}

		{
			EditorMenu* windowMenu = new EditorMenu("Window");
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Particle_View, std::string(), particleViewportPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(EditorWindowType::Particle_Inspector, std::string(), particleInspectorPanel->GetShowPtr()));
			PushMenu(windowMenu);
		}

		PushPanel(particleViewportPanel);
		PushPanel(particleInspectorPanel);
	}

	void ParticleEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorWindowType::Particle_View).c_str(), dockLeft);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorWindowType::Particle_Inspector).c_str(), dockRight);
	}

}
