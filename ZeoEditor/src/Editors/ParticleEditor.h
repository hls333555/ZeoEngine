#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class ParticleEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

	private:
		virtual Scope<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<Scene> CreateScene() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

	public:
		virtual AssetHandle<IAsset> GetAsset() const override;
		virtual AssetTypeId GetAssetTypeId() const override;
	private:
		virtual void LoadAsset(const std::string& path) override;
		virtual void LoadAndApplyDefaultAsset() override;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

		void ReloadParticleTemplateData();
	};

}
