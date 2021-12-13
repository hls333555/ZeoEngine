#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class MaterialEditor;

	class MaterialEditorSceneRenderer : public SceneRenderer
	{
	public:
		explicit MaterialEditorSceneRenderer(const Ref<MaterialEditor>& materialEditor);

	private:
		virtual Ref<FrameBuffer> CreateFrameBuffer() override;
		virtual Scope<RenderGraph> CreateRenderGraph(const Ref<FrameBuffer>& fbo) override;
		virtual Scope<RenderSystemBase> CreateRenderSystem() override;

		virtual void OnRenderScene() override;

	private:
		Ref<MaterialEditor> m_MaterialEditor;
	};

}
