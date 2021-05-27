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

	AssetTypeId ParticleEditor::GetAssetTypeId() const
	{
		return AssetType<ParticleTemplate>::Id();
	}

	void ParticleEditor::Serialize(const std::string& filePath)
	{
		auto& particlePreviewComp = GetContextEntity().GetComponent<ParticleSystemPreviewComponent>();
		AssetSerializer::Serialize<ParticleTemplate>(filePath, particlePreviewComp);
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
