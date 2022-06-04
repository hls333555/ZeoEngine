#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class LevelEditor;

	class LevelEditorSceneRenderer : public SceneRenderer
	{
	public:
		explicit LevelEditorSceneRenderer(const Ref<LevelEditor>& levelEditor);

	private:
		virtual Scope<RenderGraph> CreateRenderGraph() override;
		virtual Scope<RenderSystemBase> CreateRenderSystem(const Ref<Scene>& scene) override;

		virtual void OnRenderScene() override;

	private:
		Ref<LevelEditor> m_LevelEditor;
	};

}
