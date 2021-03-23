#include "Dockspaces/ParticleEditorDockspace.h"

#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/Components.h"
#include "Panels/ParticleViewportPanel.h"

namespace ZeoEngine {

	void ParticleEditorDockspace::OnAttach()
	{
		DockspaceBase::OnAttach();

		CreatePanel(PanelType::ParticleView);
		CreatePanel(PanelType::ParticleInspector);

		CreateMenu("File")
			.MenuItem<MenuItem_NewScene>(ICON_FA_FILE "  New particle template", "CTRL+N")
			.MenuItem<MenuItem_OpenScene>(ICON_FA_FILE_IMPORT "  Open particle template", "CTRL+O")
			.MenuItem<MenuItem_SaveScene>(ICON_FA_SAVE "  Save particle template", "CTRL+S")
			.MenuItem<MenuItem_SaveSceneAs>(ICON_FA_SAVE "  Save particle template As", "CTRL+ALT+S");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y")
			.MenuItem<MenuItem_Snapshot>(ICON_FA_CAMERA "  Snapshot");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel>(PanelType::ParticleView)
			.MenuItem<MenuItem_TogglePanel>(PanelType::ParticleInspector);

		// When a new scene is created, all previous particle's changes should be discarded
		m_PreSceneCreate.connect<&ParticleEditorDockspace::ReloadParticleTemplateData>(this);
	}

	void ParticleEditorDockspace::ReloadParticleTemplateData()
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([](auto& pspc)
		{
			ParticleLibrary::Get().ReloadAsset(pspc.Template);
		});
	}

	void ParticleEditorDockspace::Serialize(const std::string& filePath)
	{
		TypeSerializer serializer(filePath);
		auto& pspc = GetContextEntity().GetComponent<ParticleSystemPreviewComponent>();
		// Only snapshot on save when thumbnail texture is null
		if (!pspc.Template->PreviewThumbnail)
		{
			ParticleViewportPanel* particleViewportPanel = GetPanel<ParticleViewportPanel>(PanelType::ParticleView);
			std::string snapshotName = filePath + ".png";
			particleViewportPanel->Snapshot(snapshotName, 256);
		}
		serializer.Serialize(pspc, GetAssetType());
	}

	void ParticleEditorDockspace::Deserialize(const std::string& filePath)
	{
		// Deserialize particle template data and create particle system instance
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&filePath](auto& pspc)
		{
			const auto& pTemplate = ParticleLibrary::Get().GetOrLoadAsset(filePath);
			pspc.SetTemplate(pTemplate);
		});
	}

	void ParticleEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow(GetPanelName(PanelType::ParticleView), dockLeft);
		ImGui::DockBuilderDockWindow(GetPanelName(PanelType::ParticleInspector), dockRight);
	}

}
