#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class TextureEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

	private:
		virtual Scope<EditorUIRendererBase> CreateEditorUIRenderer() override;

	public:
		virtual AssetHandle<IAsset> GetAsset() const override;
		virtual AssetTypeId GetAssetTypeId() const override;
	private:
		virtual void LoadAsset(const std::string& path) override;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

	};

}
