#include "Dockspaces/ParticleEditorDockspace.h"

#include <imgui_internal.h>

#include "Panels/ParticleViewportPanel.h"

#define PARTICLE_VIEW_NAME "Particle View"
#define PARTICLE_INSPECTOR_NAME "Particle Inspector"

namespace ZeoEngine {

	ParticleEditorDockspace::ParticleEditorDockspace(const std::string& dockspaceName, float dockspaceRounding, float dockspaceBorderSize, ImVec2 dockspacePadding, ImGuiWindowFlags dockspaceWindowFlags, ImVec2 dockspacePos, ImVec2 dockspaceSize)
		: EditorDockspace(dockspaceName, dockspaceRounding, dockspaceBorderSize, dockspacePadding, dockspaceWindowFlags, dockspacePos, dockspaceSize)
	{
	}

	void ParticleEditorDockspace::OnAttach()
	{
		EditorDockspace::OnAttach();

		PushPanel(new ParticleViewportPanel(PARTICLE_VIEW_NAME, this, true));
	}

	void ParticleEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow(PARTICLE_VIEW_NAME, dockLeft);
		ImGui::DockBuilderDockWindow(PARTICLE_INSPECTOR_NAME, dockRight);
	}

}
