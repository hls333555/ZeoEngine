#include "SceneRenderers/ParticleEditorSceneRenderer.h"

#include "Editors/ParticleEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	ParticleEditorSceneRenderer::ParticleEditorSceneRenderer(const Ref<ParticleEditor>& particleEditor)
		: m_ParticleEditor(particleEditor)
	{
	}

	Ref<FrameBuffer> ParticleEditorSceneRenderer::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ FrameBufferTextureFormat::RGBA8, { FrameBufferSamplerType::BilinearClamp } },
			{ FrameBufferTextureFormat::DEPTH24STENCIL8, { FrameBufferSamplerType::BilinearClamp } }
		};
		return FrameBuffer::Create(fbSpec);
	}

	Scope<RenderGraph> ParticleEditorSceneRenderer::CreateRenderGraph(const Ref<FrameBuffer>& fbo)
	{
		return CreateScope<ForwardRenderGraph>(fbo);
	}

	Scope<RenderSystemBase> ParticleEditorSceneRenderer::CreateRenderSystem()
	{
		return CreateScope<ParticlePreviewRenderSystem>(shared_from_this());
	}

	void ParticleEditorSceneRenderer::OnRenderScene()
	{
		BeginScene(*m_ParticleEditor->GetEditorCamera());
		GetRenderSystem()->OnRenderEditor();
		EndScene();
	}

}
