#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class MeshEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

	private:
		virtual Scope<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

	public:
		virtual Ref<IAsset> GetAsset() const override;
	private:
		virtual void LoadAsset(const std::filesystem::path& path) override;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

	};

}
