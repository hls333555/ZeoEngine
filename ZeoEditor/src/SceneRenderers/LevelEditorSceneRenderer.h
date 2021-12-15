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
		virtual Ref<FrameBuffer> CreateFrameBuffer() override;
		virtual Scope<RenderGraph> CreateRenderGraph(const Ref<FrameBuffer>& fbo) override;
		virtual Scope<RenderSystemBase> CreateRenderSystem() override;

		virtual void Prepare() override;
		virtual void OnRenderScene() override;

	private:
		Ref<LevelEditor> m_LevelEditor;
	};

}
