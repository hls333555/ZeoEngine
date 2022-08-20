#include "Editors/TextureEditor.h"

#include "Engine/GameFramework/Components.h"
#include "EditorUIRenderers/TextureEditorUIRenderer.h"
#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	Scope<EditorUIRendererBase> TextureEditor::CreateEditorUIRenderer()
	{
		return CreateScope<TextureEditorUIRenderer>(SharedFromBase<TextureEditor>());
	}

	Ref<IAsset> TextureEditor::GetAsset() const
	{
		return GetContextEntity().GetComponent<TexturePreviewComponent>().TextureAsset;
	}

	// TODO: Should support reloading
	void TextureEditor::LoadAsset(const std::string& path)
	{
		GetContextEntity().PatchComponent<TexturePreviewComponent>([&path, this](auto& texturePreviewComp)
		{
			texturePreviewComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>(path);
		});
	}

	void TextureEditor::SaveAsset(const std::string& path)
	{
		AssetManager::Get().SaveAsset(path, GetAsset());
		GetContextEntity().GetComponent<TexturePreviewComponent>().TextureAsset = AssetLibrary::LoadAsset<Texture2D>(path, AssetLibrary::DeserializeMode::Force);
	}

	Entity TextureEditor::CreatePreviewEntity(const Ref<Scene>& scene)
	{
		Entity previewTextureEntity = scene->CreateEntity("Preview Texture");
		previewTextureEntity.AddComponent<TexturePreviewComponent>();
		return previewTextureEntity;
	}

}
