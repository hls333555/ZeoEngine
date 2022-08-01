#include "Editors/MeshEditor.h"

#include "Engine/Renderer/EditorCamera.h"
#include "EditorUIRenderers/MeshEditorUIRenderer.h"
#include "SceneRenderers/MeshEditorSceneRenderer.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	Scope<EditorUIRendererBase> MeshEditor::CreateEditorUIRenderer()
	{
		return CreateScope<MeshEditorUIRenderer>(SharedFromBase<MeshEditor>());
	}

	Ref<SceneRenderer> MeshEditor::CreateSceneRenderer()
	{
		return CreateRef<MeshEditorSceneRenderer>(SharedFromBase<MeshEditor>());
	}

	Ref<IAsset> MeshEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<MeshPreviewComponent>().MeshAsset;
	}

	// TODO: Should support reloading
	void MeshEditor::LoadAsset(const std::filesystem::path& path)
	{
		GetContextEntity().PatchComponent<MeshPreviewComponent>([&path, this](auto& meshPreviewComp)
		{
			// When load asset the second time, the deserialization will never happen, so we have to update mesh asset here
			meshPreviewComp.MeshAsset = AssetLibrary::LoadAsset<Mesh>(path, &meshPreviewComp);
			meshPreviewComp.Instance = meshPreviewComp.MeshAsset->CreateInstance(GetContextEntity().GetScene());
			meshPreviewComp.Instance->SubmitAllTechniques();
		});
		GetContextEntity().UpdateBounds();
		FocusContextEntity();
	}

	Entity MeshEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewMeshEntity = scene->CreateEntity("Preview Mesh");
		previewMeshEntity.AddComponent<MeshPreviewComponent>();
		previewMeshEntity.AddComponent<LightComponent>(LightComponent::LightType::DirectionalLight);
		return previewMeshEntity;
	}
	
}
