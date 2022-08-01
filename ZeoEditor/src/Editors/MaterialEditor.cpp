#include "Editors/MaterialEditor.h"

#include "Engine/Renderer/EditorCamera.h"
#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/MaterialEditorUIRenderer.h"
#include "SceneRenderers/MaterialEditorSceneRenderer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	Scope<EditorUIRendererBase> MaterialEditor::CreateEditorUIRenderer()
	{
		return CreateScope<MaterialEditorUIRenderer>(SharedFromBase<MaterialEditor>());
	}

	Ref<SceneRenderer> MaterialEditor::CreateSceneRenderer()
	{
		return CreateRef<MaterialEditorSceneRenderer>(SharedFromBase<MaterialEditor>());
	}

	Ref<IAsset> MaterialEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<MaterialPreviewComponent>().MaterialAsset;
	}

	// TODO: Should support reloading
	void MaterialEditor::LoadAsset(const std::filesystem::path& path)
	{
		GetContextEntity().PatchComponent<MaterialPreviewComponent>([&path, this](auto& materialPreviewComp)
		{
			materialPreviewComp.MaterialAsset = AssetLibrary::LoadAsset<Material>(path);
			const auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
			meshComp.Instance->SetMaterial(0, materialPreviewComp.MaterialAsset);
		});
	}

	void MaterialEditor::LoadAndApplyDefaultAsset()
	{
		GetContextEntity().PatchComponent<MaterialPreviewComponent>([this](auto& materialPreviewComp)
		{
			materialPreviewComp.MaterialAsset = Material::GetDefaultMaterial();
			const auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
			meshComp.Instance->SetMaterial(0, materialPreviewComp.MaterialAsset);
		});
		FocusContextEntity();
	}

	Entity MaterialEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewMaterialEntity = scene->CreateEntity("Preview Material");
		previewMaterialEntity.AddComponent<MaterialPreviewComponent>();
		previewMaterialEntity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultSphereMesh());
		previewMaterialEntity.AddComponent<LightComponent>(LightComponent::LightType::DirectionalLight);
		return previewMaterialEntity;
	}

}
