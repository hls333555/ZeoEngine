#include "Editors/ParticleEditor.h"

#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/ParticleEditorUIRenderer.h"
#include "SceneRenderers/ParticleEditorSceneRenderer.h"
#include "Scenes/ParticleEditorScene.h"
#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	void ParticleEditor::OnAttach()
	{
		// TODO: If no change was made, no need to reload
		// When a new scene is created, all previous particle's changes should be discarded
		m_PreSceneCreate.connect<&ParticleEditor::ReloadParticleTemplateData>(this);

		EditorBase::OnAttach();
	}

	Scope<EditorUIRendererBase> ParticleEditor::CreateEditorUIRenderer()
	{
		return CreateScope<ParticleEditorUIRenderer>(SharedFromBase<ParticleEditor>());
	}

	Ref<Scene> ParticleEditor::CreateScene()
	{
		return CreateRef<ParticleEditorScene>();
	}

	Ref<SceneRenderer> ParticleEditor::CreateSceneRenderer()
	{
		return CreateScope<ParticleEditorSceneRenderer>(SharedFromBase<ParticleEditor>());
	}

	Ref<IAsset> ParticleEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<ParticleSystemPreviewComponent>().ParticleTemplateAsset;
	}

	void ParticleEditor::LoadAsset(const std::string& path)
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&path](auto& particlePreviewComp)
		{
			particlePreviewComp.ParticleTemplateAsset = AssetLibrary::LoadAsset<ParticleTemplate>(path);
		});
	}

	void ParticleEditor::SaveAsset(const std::string& path)
	{
		AssetManager::Get().SaveAsset(path, GetAsset());
		GetContextEntity().GetComponent<ParticleSystemPreviewComponent>().ParticleTemplateAsset = AssetLibrary::LoadAsset<ParticleTemplate>(path, AssetLibrary::DeserializeMode::Force);
	}

	void ParticleEditor::LoadAndApplyDefaultAsset()
	{
		GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([](auto& particlePreviewComp)
		{
			particlePreviewComp.ParticleTemplateAsset = AssetLibrary::LoadAsset<ParticleTemplate>(ParticleTemplate::GetTemplatePath());
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

	void ParticleEditor::ReloadParticleTemplateData() const
	{
		auto previewParticleEntity = GetContextEntity();
		if (!previewParticleEntity) return;
		
		previewParticleEntity.PatchComponent<ParticleSystemPreviewComponent>([](auto& particlePreviewComp)
		{
			AssetLibrary::ReloadAsset(particlePreviewComp.ParticleTemplateAsset->GetHandle());
		});
	}

}
