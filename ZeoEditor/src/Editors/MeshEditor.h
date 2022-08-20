#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class Mesh;
	struct MeshPreviewComponent;

	class MeshEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

	private:
		virtual Scope<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

	public:
		virtual Ref<IAsset> GetAsset() const override;
	private:
		virtual void LoadAsset(const std::string& path) override;
		virtual void SaveAsset(const std::string& path) override;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;
		void SetMeshAsset(MeshPreviewComponent& meshPreviewComp, Ref<Mesh> mesh) const;

	};

}
