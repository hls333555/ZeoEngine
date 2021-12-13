#include "SceneRenderers/MaterialEditorSceneRenderer.h"

#include "Editors/MaterialEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	MaterialEditorSceneRenderer::MaterialEditorSceneRenderer(const Ref<MaterialEditor>& materialEditor)
		: m_MaterialEditor(materialEditor)
	{
	}

	Ref<FrameBuffer> MaterialEditorSceneRenderer::CreateFrameBuffer()
	{
		// TODO:
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RGBA16F, FrameBufferTextureFormat::Depth };
		return FrameBuffer::Create(fbSpec);
	}

	Scope<RenderGraph> MaterialEditorSceneRenderer::CreateRenderGraph(const Ref<FrameBuffer>& fbo)
	{
		return CreateScope<ForwardRenderGraph>(fbo);
	}

	Scope<RenderSystemBase> MaterialEditorSceneRenderer::CreateRenderSystem()
	{
		return CreateScope<MaterialPreviewRenderSystem>(m_MaterialEditor->GetScene(), shared_from_this());
	}

	void MaterialEditorSceneRenderer::OnRenderScene()
	{
		BeginScene(*m_MaterialEditor->GetEditorCamera());
		GetRenderSystem()->OnRenderEditor();
		EndScene();
	}

}
