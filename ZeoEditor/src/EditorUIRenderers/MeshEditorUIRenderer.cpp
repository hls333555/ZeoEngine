#include "EditorUIRenderers/MeshEditorUIRenderer.h"

#include <imgui_internal.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Panels/MeshEditorViewPanel.h"
#include "Panels/InspectorPanels.h"

namespace ZeoEngine {

	void MeshEditorUIRenderer::OnAttach()
	{
		EditorUIRendererBase::OnAttach();

		CreatePanel<MeshEditorViewPanel>(MESH_EDITOR_VIEW);
		CreatePanel<MeshInspectorPanel>(MESH_INSPECTOR);

		CreateMenu("File")
			.MenuItem<MenuItem_LoadAsset>(ICON_FA_FILE_IMPORT "  Load mesh", "CTRL+O")
			.MenuItem<MenuItem_SaveAsset>(ICON_FA_SAVE "  Save mesh", "CTRL+S")
			.MenuItem<MenuItem_SaveAssetAs>(ICON_FA_SAVE "  Save mesh As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel<MeshEditorViewPanel>>(MESH_EDITOR_VIEW)
			.MenuItem<MenuItem_TogglePanel<MeshInspectorPanel>>(MESH_INSPECTOR);

	}

	void MeshEditorUIRenderer::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow("###" MESH_EDITOR_VIEW, dockLeft);
		ImGui::DockBuilderDockWindow(MESH_INSPECTOR, dockRight);
	}

}
