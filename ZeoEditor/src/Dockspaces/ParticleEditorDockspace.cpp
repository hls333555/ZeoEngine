#include "Dockspaces/ParticleEditorDockspace.h"

#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	void ParticleEditorDockspace::OnAttach()
	{
		DockspaceBase::OnAttach();

		CreatePanel(PanelType::ParticleView);
		CreatePanel(PanelType::ParticleInspector);

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
			.MenuItem<MenuItem_TogglePanel>(PanelType::ParticleView)
			.MenuItem<MenuItem_TogglePanel>(PanelType::ParticleInspector);
		
	}

	void ParticleEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::ParticleView), dockLeft);
		ImGui::DockBuilderDockWindow(EditorUtils::GetPanelName(PanelType::ParticleInspector), dockRight);
	}

}
