#include "Editors/TextureEditor.h"

#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/TextureEditorUIRenderer.h"

namespace ZeoEngine {

	Scope<EditorUIRendererBase> TextureEditor::CreateEditorUIRenderer()
	{
		return CreateScope<TextureEditorUIRenderer>(SharedFromBase<TextureEditor>());
	}

	AssetTypeId TextureEditor::GetAssetTypeId() const
	{
		return Texture2D::TypeId();
	}

	AssetHandle<IAsset> TextureEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<TexturePreviewComponent>().TextureAsset;
	}

	// TODO: Should support reloading
	void TextureEditor::LoadAsset(const std::string& path)
	{
		GetContextEntity().PatchComponent<TexturePreviewComponent>([&path, this](auto& texturePreviewComp)
		{
			texturePreviewComp.TextureAsset = Texture2DLibrary::Get().LoadAsset(path);
		});
	}

	Entity TextureEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewTextureEntity = scene->CreateEntity("Preview Texture");
		previewTextureEntity.AddComponent<TexturePreviewComponent>();
		return previewTextureEntity;
	}

}
