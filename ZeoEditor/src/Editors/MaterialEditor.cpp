#include "Editors/MaterialEditor.h"

#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/MaterialEditorUIRenderer.h"
#include "Scenes/MaterialEditorScene.h"
#include "SceneRenderers/MaterialEditorSceneRenderer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	void MaterialEditor::OnAttach()
	{
		EditorBase::OnAttach();

		GetEditorCamera()->SetDistance(2.0f);
	}

	Ref<EditorUIRendererBase> MaterialEditor::CreateEditorUIRenderer()
	{
		return CreateRef<MaterialEditorUIRenderer>(SharedFromBase<MaterialEditor>());
	}

	Ref<Scene> MaterialEditor::CreateScene()
	{
		return CreateRef<MaterialEditorScene>();
	}

	Ref<SceneRenderer> MaterialEditor::CreateSceneRenderer()
	{
		return CreateRef<MaterialEditorSceneRenderer>(SharedFromBase<MaterialEditor>());
	}

	AssetTypeId MaterialEditor::GetAssetTypeId() const
	{
		return Material::TypeId();
	}

	AssetHandle<IAsset> MaterialEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<MaterialPreviewComponent>().MaterialAsset;
	}

	void MaterialEditor::LoadAsset(const std::string& path)
	{
		GetContextEntity().PatchComponent<MaterialPreviewComponent>([&path, this](auto& materialPreviewComp)
		{
			materialPreviewComp.MaterialAsset = MaterialLibrary::Get().LoadAsset(path);
			const auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
			meshComp.Instance->SetMaterial(0, materialPreviewComp.MaterialAsset);
		});
	}

	void MaterialEditor::LoadAndApplyDefaultAsset()
	{
		GetContextEntity().PatchComponent<MaterialPreviewComponent>([this](auto& materialPreviewComp)
		{
			materialPreviewComp.MaterialAsset = MaterialLibrary::GetDefaultMaterial();
			const auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
			meshComp.Instance->SetMaterial(0, materialPreviewComp.MaterialAsset);
		});
	}

	Entity MaterialEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewMaterialEntity = scene->CreateEntity("Preview Material");
		previewMaterialEntity.AddComponent<MaterialPreviewComponent>();
		previewMaterialEntity.AddComponent<MeshRendererComponent>(MeshLibrary::GetDefaultSphereMesh());
		previewMaterialEntity.AddComponent<LightComponent>(LightComponent::LightType::DirectionalLight);
		return previewMaterialEntity;
	}

}
