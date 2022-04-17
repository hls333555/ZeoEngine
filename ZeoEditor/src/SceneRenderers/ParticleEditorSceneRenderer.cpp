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
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
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
