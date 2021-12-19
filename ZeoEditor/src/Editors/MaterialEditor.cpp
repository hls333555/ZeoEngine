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
		m_PostSceneCreate.connect<&MaterialEditor::CreatePreviewMaterial>(this);

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
		return MaterialAsset::TypeId();
	}

	AssetHandle<IAsset> MaterialEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<MaterialPreviewComponent>().Template;
	}

	void MaterialEditor::LoadAsset(const std::string& path)
	{
		GetContextEntity().PatchComponent<MaterialPreviewComponent>([&path, this](auto& materialPreviewComp)
		{
			materialPreviewComp.Template = MaterialAssetLibrary::Get().LoadAsset(path);
			auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
			meshComp.Instance->SetMaterial(0, materialPreviewComp.Template);
		});
	}

	void MaterialEditor::SaveAsset(const std::string& path)
	{
		auto& materialPreviewComp = GetContextEntity().GetComponent<MaterialPreviewComponent>();
		materialPreviewComp.Template->Serialize(path);
	}

	void MaterialEditor::CreatePreviewMaterial(const Ref<Scene>& scene, bool bIsFromLoad)
	{
		Entity previewMaterialEntity = scene->CreateEntity("Preview Material");
		previewMaterialEntity.AddComponent<MaterialPreviewComponent>();
		previewMaterialEntity.AddComponent<MeshRendererComponent>(MeshAssetLibrary::GetDefaultSphereMesh());
		previewMaterialEntity.AddComponent<LightComponent>(LightComponent::LightType::DirectionalLight);
		SetContextEntity(previewMaterialEntity);
	}

}
