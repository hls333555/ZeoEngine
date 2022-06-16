#include "EditorUIRenderers/TextureEditorUIRenderer.h"

#include <imgui_internal.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Panels/TextureEditorViewPanel.h"
#include "Panels/InspectorPanels.h"

namespace ZeoEngine {

	void TextureEditorUIRenderer::OnAttach()
	{
		EditorUIRendererBase::OnAttach();

		CreatePanel<TextureEditorViewPanel>(TEXTURE_EDITOR_VIEW);
		CreatePanel<TextureInspectorPanel>(TEXTURE_INSPECTOR);

		CreateMenu("File")
			.MenuItem<MenuItem_LoadAsset>(ICON_FA_FILE_IMPORT "  Load texture", "CTRL+O")
			.MenuItem<MenuItem_SaveAsset>(ICON_FA_SAVE "  Save texture", "CTRL+S")
			.MenuItem<MenuItem_SaveAssetAs>(ICON_FA_SAVE "  Save texture As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel<TextureEditorViewPanel>>(TEXTURE_EDITOR_VIEW)
			.MenuItem<MenuItem_TogglePanel<TextureInspectorPanel>>(TEXTURE_INSPECTOR);

	}

	void TextureEditorUIRenderer::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow("###" TEXTURE_EDITOR_VIEW, dockLeft);
		ImGui::DockBuilderDockWindow(TEXTURE_INSPECTOR, dockRight);
	}

}
