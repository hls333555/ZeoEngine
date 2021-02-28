#include "Dockspaces/ParticleEditorDockspace.h"

#include <imgui_internal.h>

#include "Panels/ParticleViewportPanel.h"
#include "Panels/DataInspectorPanel.h"
#include "Menus/EditorMenuItem.h"
#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void ParticleEditorDockspace::OnAttach()
	{
		EditorDockspace::OnAttach();

		ParticleViewportPanel* particleViewportPanel = new ParticleViewportPanel(EditorPanelType::Particle_View, this, true);
		ParticleInspectorPanel* particleInspectorPanel = new ParticleInspectorPanel(EditorPanelType::Particle_Inspector, this, true);

		{
			EditorMenu* fileMenu = new EditorMenu("File");
			fileMenu->PushMenuItem(new MenuItem_NewScene("New particle template", "CTRL+N"));
			fileMenu->PushMenuItem(new MenuItem_OpenScene("Open particle template", "CTRL+O"));
			fileMenu->PushMenuItem(new MenuItem_SaveScene("Save particle template", "CTRL+S"));
			fileMenu->PushMenuItem(new MenuItem_SaveSceneAs("Save particle template As", "CTRL+ALT+S"));
			PushMenu(fileMenu);
		}

		{
			EditorMenu* editMenu = new EditorMenu("Edit");
			editMenu->PushMenuItem(new MenuItem_Undo("Undo", "CTRL+Z"));
			editMenu->PushMenuItem(new MenuItem_Redo("Redo", "CTRL+Y"));
			editMenu->PushMenuItem(new MenuItem_Snapshot("Snapshot", ""));
			PushMenu(editMenu);
		}

		{
			EditorMenu* windowMenu = new EditorMenu("Window");
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Particle_View), "", particleViewportPanel->GetShowPtr()));
			windowMenu->PushMenuItem(new MenuItem_ToggleWindow(ResolveEditorNameFromEnum(EditorPanelType::Particle_Inspector), "", particleInspectorPanel->GetShowPtr()));
			PushMenu(windowMenu);
		}

		PushPanel(particleViewportPanel);
		PushPanel(particleInspectorPanel);
	}

	void ParticleEditorDockspace::CreateNewScene(bool bIsFromOpenScene)
	{
		// Reload particle template data
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([](auto& pspc)
		{
			ParticleLibrary::Get().Reload(pspc.Template);
		});

		EditorDockspace::CreateNewScene(bIsFromOpenScene);
	}

	void ParticleEditorDockspace::Serialize(const std::string& filePath)
	{
		TypeSerializer serializer(filePath);
		auto& pspc = m_ContextEntity.GetComponent<ParticleSystemPreviewComponent>();
		// Only snapshot on save when thumbnail texture is null
		if (!pspc.Template->PreviewThumbnail)
		{
			ParticleViewportPanel* viewportPanel = GetPanelByType<ParticleViewportPanel>(EditorPanelType::Particle_View);
			std::string snapshotName = filePath + ".png";
			viewportPanel->Snapshot(snapshotName, 256);
		}
		serializer.Serialize(pspc, GetAssetType());
	}

	void ParticleEditorDockspace::Deserialize(const std::string& filePath)
	{
		// Deserialize particle template data and create particle system instance
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&filePath](auto& pspc)
		{
			const auto& pTemplate = ParticleLibrary::Get().GetOrLoad(filePath);
			pspc.SetTemplate(pTemplate);
		});
	}

	void ParticleEditorDockspace::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.5f, nullptr, &dockLeft);

		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorPanelType::Particle_View).c_str(), dockLeft);
		ImGui::DockBuilderDockWindow(ResolveEditorNameFromEnum(EditorPanelType::Particle_Inspector).c_str(), dockRight);
	}

}
