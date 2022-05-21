#include "Editors/ParticleEditor.h"

#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/ParticleEditorUIRenderer.h"
#include "SceneRenderers/ParticleEditorSceneRenderer.h"
#include "Scenes/ParticleEditorScene.h"

namespace ZeoEngine {

	void ParticleEditor::OnAttach()
	{
		// TODO: If no change was made, no need to reload
		// When a new scene is created, all previous particle's changes should be discarded
		m_PreSceneCreate.connect<&ParticleEditor::ReloadParticleTemplateData>(this);

		EditorBase::OnAttach();
	}

	Ref<EditorUIRendererBase> ParticleEditor::CreateEditorUIRenderer()
	{
		return CreateRef<ParticleEditorUIRenderer>(SharedFromBase<ParticleEditor>());
	}

	Ref<Scene> ParticleEditor::CreateScene()
	{
		return CreateRef<ParticleEditorScene>();
	}

	Ref<SceneRenderer> ParticleEditor::CreateSceneRenderer()
	{
		return CreateScope<ParticleEditorSceneRenderer>(SharedFromBase<ParticleEditor>());
	}

	AssetTypeId ParticleEditor::GetAssetTypeId() const
	{
		return ParticleTemplate::TypeId();
	}

	AssetHandle<IAsset> ParticleEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<ParticleSystemPreviewComponent>().ParticleTemplateAsset;
	}

	void ParticleEditor::LoadAsset(const std::string& path)
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&path](auto& particlePreviewComp)
		{
			particlePreviewComp.ParticleTemplateAsset = ParticleTemplateLibrary::Get().LoadAsset(path);
		});
	}

	void ParticleEditor::LoadAndApplyDefaultAsset()
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([](auto& particlePreviewComp)
		{
			particlePreviewComp.ParticleTemplateAsset = ParticleTemplateLibrary::GetDefaultParticleTemplate();
		});
	}

	Entity ParticleEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewParticleEntity = scene->CreateEntity("Preview Particle");
		auto& particlePreviewComp = previewParticleEntity.AddComponent<ParticleSystemPreviewComponent>();
		// TODO:
		// For loading phase, instance will be created in ParticleScene::PostLoad
		ParticleSystemInstance::Create(particlePreviewComp);
		return previewParticleEntity;
	}

	void ParticleEditor::ReloadParticleTemplateData()
	{
		auto previewParticleEntity = GetContextEntity();
		if (!previewParticleEntity) return;
		
		previewParticleEntity.PatchComponent<ParticleSystemPreviewComponent>([](auto& particlePreviewComp)
		{
			ParticleTemplateLibrary::Get().ReloadAsset(particlePreviewComp.ParticleTemplateAsset->GetID());
		});
	}

}
