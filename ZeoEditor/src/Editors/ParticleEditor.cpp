#include "Editors/ParticleEditor.h"

#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/Components.h"
#include "Dockspaces/DockspaceBase.h"
#include "Panels/ParticleViewportPanel.h"

namespace ZeoEngine {

	void ParticleEditor::OnAttach()
	{
		EditorBase::OnAttach();

		// When a new scene is created, all previous particle's changes should be discarded
		m_PreSceneCreate.connect<&ParticleEditor::ReloadParticleTemplateData>(this);
	}

	void ParticleEditor::Serialize(const std::string& filePath)
	{
		auto& particlePreviewComp = GetContextEntity().GetComponent<ParticleSystemPreviewComponent>();
		// Only snapshot on save when thumbnail texture is null
		if (!particlePreviewComp.Template->PreviewThumbnail)
		{
			ParticleViewportPanel* particleViewportPanel = GetDockspace()->GetPanel<ParticleViewportPanel>(PanelType::ParticleView);
			std::string snapshotName = filePath + ".png";
			particleViewportPanel->Snapshot(snapshotName, 256);
		}
		AssetSerializer::Serialize(filePath, GetAssetType(), particlePreviewComp);
	}

	void ParticleEditor::Deserialize(const std::string& filePath)
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&filePath](auto& particlePreviewComp)
		{
			particlePreviewComp.Template = ParticleLibrary::Get().LoadAsset(filePath);
		});
	}

	void ParticleEditor::ReloadParticleTemplateData()
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([](auto& particlePreviewComp)
		{
			ParticleLibrary::Get().ReloadAsset(particlePreviewComp.Template->GetPath());
		});
	}

}
