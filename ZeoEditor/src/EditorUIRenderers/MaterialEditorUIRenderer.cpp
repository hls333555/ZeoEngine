#include "EditorUIRenderers/MaterialEditorUIRenderer.h"

#include <imgui_internal.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Panels/MaterialEditorViewPanel.h"
#include "Panels/InspectorPanels.h"

namespace ZeoEngine {

	void MaterialEditorUIRenderer::OnAttach()
	{
		EditorUIRendererBase::OnAttach();

		CreatePanel<MaterialEditorViewPanel>(MATERIAL_EDITOR_VIEW);
		CreatePanel<MaterialInspectorPanel>(MATERIAL_INSPECTOR);

		CreateMenu("File")
			.MenuItem<MenuItem_NewAsset>(ICON_FA_FILE "  New material", "CTRL+N")
			.MenuItem<MenuItem_LoadAsset>(ICON_FA_FILE_IMPORT "  Load material", "CTRL+O")
			.MenuItem<MenuItem_SaveAsset>(ICON_FA_SAVE "  Save material", "CTRL+S")
			.MenuItem<MenuItem_SaveAssetAs>(ICON_FA_SAVE "  Save material As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y")
			.MenuItem<MenuItem_Snapshot>(ICON_FA_CAMERA "  Snapshot");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel<MaterialEditorViewPanel>>(MATERIAL_EDITOR_VIEW)
			.MenuItem<MenuItem_TogglePanel<MaterialInspectorPanel>>(MATERIAL_INSPECTOR);
		
	}

	void MaterialEditorUIRenderer::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow("###" MATERIAL_EDITOR_VIEW, dockLeft);
		ImGui::DockBuilderDockWindow(MATERIAL_INSPECTOR, dockRight);
	}

}
