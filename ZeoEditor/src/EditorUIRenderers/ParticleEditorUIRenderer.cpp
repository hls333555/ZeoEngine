#include "EditorUIRenderers/ParticleEditorUIRenderer.h"

#include <imgui_internal.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Panels/ParticleViewPanel.h"
#include "Panels/InspectorPanels.h"

namespace ZeoEngine {

	void ParticleEditorUIRenderer::OnAttach()
	{
		EditorUIRendererBase::OnAttach();

		CreatePanel<ParticleViewPanel>(PARTICLE_VIEW);
		CreatePanel<ParticleInspectorPanel>(PARTICLE_INSPECTOR);

		CreateMenu("File")
			.MenuItem<MenuItem_NewAsset>(ICON_FA_FILE "  New particle template", "CTRL+N")
			.MenuItem<MenuItem_LoadAsset>(ICON_FA_FILE_IMPORT "  Load particle template", "CTRL+O")
			.MenuItem<MenuItem_SaveAsset>(ICON_FA_SAVE "  Save particle template", "CTRL+S")
			.MenuItem<MenuItem_SaveAssetAs>(ICON_FA_SAVE "  Save particle template As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y")
			.MenuItem<MenuItem_Snapshot>(ICON_FA_CAMERA "  Snapshot");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel<ParticleViewPanel>>(PARTICLE_VIEW)
			.MenuItem<MenuItem_TogglePanel<ParticleInspectorPanel>>(PARTICLE_INSPECTOR);
		
	}

	void ParticleEditorUIRenderer::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow("###" PARTICLE_VIEW, dockLeft);
		ImGui::DockBuilderDockWindow(PARTICLE_INSPECTOR, dockRight);
	}

}
