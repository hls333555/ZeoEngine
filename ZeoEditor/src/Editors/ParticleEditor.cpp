#include "Editors/ParticleEditor.h"

#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/ParticleEditorUIRenderer.h"
#include "Scenes/ParticleScene.h"

namespace ZeoEngine {

	void ParticleEditor::OnAttach()
	{
		EditorBase::OnAttach();

		// TODO: If no change was made, no need to reload
		// When a new scene is created, all previous particle's changes should be discarded
		m_PreSceneCreate.connect<&ParticleEditor::ReloadParticleTemplateData>(this);
	}

	AssetTypeId ParticleEditor::GetAssetTypeId() const
	{
		return ParticleTemplateAsset::TypeId();
	}

	std::string ParticleEditor::GetAssetPath() const
	{
		return GetContextEntity().GetComponent<ParticleSystemPreviewComponent>().Template->GetPath();
	}

	Ref<EditorUIRendererBase> ParticleEditor::CreateEditorUIRenderer()
	{
		return CreateRef<ParticleEditorUIRenderer>(shared_from_this());
	}

	Ref<Scene> ParticleEditor::CreateScene()
	{
		return CreateRef<ParticleScene>();
	}

	void ParticleEditor::LoadAssetImpl(const std::string& filePath)
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&filePath](auto& particlePreviewComp)
		{
			particlePreviewComp.Template = ParticleTemplateAssetLibrary::Get().LoadAsset(filePath);
		});
	}

	void ParticleEditor::SaveAssetImpl(const std::string& filePath)
	{
		auto& particlePreviewComp = GetContextEntity().GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->Serialize(filePath);
	}

	void ParticleEditor::ReloadParticleTemplateData()
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([](auto& particlePreviewComp)
		{
			ParticleTemplateAssetLibrary::Get().ReloadAsset(particlePreviewComp.Template->GetPath());
		});
	}

}
