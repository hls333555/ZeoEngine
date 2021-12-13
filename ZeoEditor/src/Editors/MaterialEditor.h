#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class MaterialEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

	private:
		virtual Ref<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<Scene> CreateScene() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

	public:
		virtual AssetHandle<IAsset> GetAsset() const override;
		virtual AssetTypeId GetAssetTypeId() const override;
	private:
		virtual void LoadAsset(const std::string& path) override;
		virtual void SaveAsset(const std::string& path) override;

		void CreatePreviewMaterial(const Ref<Scene>& scene, bool bIsFromLoad = false);

	};

}
