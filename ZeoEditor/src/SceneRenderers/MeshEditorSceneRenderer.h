#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class MeshEditor;

	class MeshEditorSceneRenderer : public SceneRenderer
	{
	public:
		explicit MeshEditorSceneRenderer(const Ref<MeshEditor>& meshEditor);

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() override;
		virtual Scope<RenderSystemBase> CreateRenderSystem(const Ref<Scene>& scene) override;

		virtual void OnRenderScene() override;

	private:
		Ref<MeshEditor> m_MeshEditor;
	};

}
