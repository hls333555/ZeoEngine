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
		virtual Ref<IAsset> GetAsset() const override;
	private:
		virtual void LoadAsset(const std::string& path) override;
		virtual void SaveAsset(const std::string& path) override;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

	};

}
