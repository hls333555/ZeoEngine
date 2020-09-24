#include "Dockspaces/ParticleEditorDockspace.h"

#include <imgui_internal.h>

#include "Panels/ParticleViewportPanel.h"
#include "Menus/EditorMenuItem.h"

#define PARTICLE_VIEW_NAME "Particle View"
#define PARTICLE_INSPECTOR_NAME "Particle Inspector"

namespace ZeoEngine {

	void ParticleEditorDockspace::OnAttach()
	{
		EditorDockspace::OnAttach();

		ParticleViewportPanel* particleViewportPanel = new ParticleViewportPanel(PARTICLE_VIEW_NAME, this, true);

		{
			EditorMenu* fileMenu = new EditorMenu("File");
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
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(PARTICLE_VIEW_NAME, std::string(), particleViewportPanel->GetShowPtr()));
			PushMenu(windowMenu);
		}

		PushPanel(particleViewportPanel);
	}

	void ParticleEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow(PARTICLE_VIEW_NAME, dockLeft);
		ImGui::DockBuilderDockWindow(PARTICLE_INSPECTOR_NAME, dockRight);
	}

}
