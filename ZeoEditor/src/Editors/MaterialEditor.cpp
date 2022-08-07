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
			SetMaterialAsset(materialPreviewComp, AssetLibrary::LoadAsset<Material>(path));
		});
	}

	void MaterialEditor::SaveAsset(const std::filesystem::path& path)
	{
		AssetManager::Get().SaveAsset(path, GetAsset());
		auto& materialPreviewComp = GetContextEntity().GetComponent<MaterialPreviewComponent>();
		// When saving material to an already loaded material asset, we have to force deserialize to reflect the difference
		auto material = AssetLibrary::LoadAsset<Material>(path, AssetLibrary::DeserializeMode::Force);
		SetMaterialAsset(materialPreviewComp, std::move(material));
	}

	void MaterialEditor::LoadAndApplyDefaultAsset()
	{
		GetContextEntity().PatchComponent<MaterialPreviewComponent>([this](auto& materialPreviewComp)
		{
			SetMaterialAsset(materialPreviewComp, Material::GetDefaultMaterial());
		});
		FocusContextEntity(true);
	}

	Entity MaterialEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewMaterialEntity = scene->CreateEntity("Preview Material");
		previewMaterialEntity.AddComponent<MaterialPreviewComponent>();
		previewMaterialEntity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultSphereMesh());
		previewMaterialEntity.AddComponent<LightComponent>(LightComponent::LightType::DirectionalLight);
		return previewMaterialEntity;
	}

	void MaterialEditor::SetMaterialAsset(MaterialPreviewComponent& materialPreviewComp, Ref<Material> material) const
	{
		materialPreviewComp.MaterialAsset = std::move(material);
		const auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
		meshComp.Instance->SetMaterial(0, materialPreviewComp.MaterialAsset);
	}

}
