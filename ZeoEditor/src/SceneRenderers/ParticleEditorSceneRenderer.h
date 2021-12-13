#pragma once

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class ParticleEditor;

	class ParticleEditorSceneRenderer : public SceneRenderer
	{
	public:
		explicit ParticleEditorSceneRenderer(const Ref<ParticleEditor>& particleEditor);

	private:
		virtual Ref<FrameBuffer> CreateFrameBuffer() override;
		virtual Scope<RenderGraph> CreateRenderGraph(const Ref<FrameBuffer>& fbo) override;
		virtual Scope<RenderSystemBase> CreateRenderSystem() override;

		virtual void OnRenderScene() override;

	private:
		Ref<ParticleEditor> m_ParticleEditor;
	};

}
