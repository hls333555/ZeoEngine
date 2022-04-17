#include "SceneRenderers/LevelEditorSceneRenderer.h"

#include "Editors/LevelEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	LevelEditorSceneRenderer::LevelEditorSceneRenderer(const Ref<LevelEditor>& levelEditor)
		: m_LevelEditor(levelEditor)
	{
	}

	Ref<FrameBuffer> LevelEditorSceneRenderer::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::RGBA16F, { SamplerType::BilinearClamp } },
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
		};
		return FrameBuffer::Create(fbSpec);
	}

	Scope<RenderGraph> LevelEditorSceneRenderer::CreateRenderGraph(const Ref<FrameBuffer>& fbo)
	{
		return CreateScope<ForwardRenderGraph>(fbo, true);
	}

	Scope<RenderSystemBase> LevelEditorSceneRenderer::CreateRenderSystem()
	{
		return CreateScope<RenderSystem>(shared_from_this());
	}

	void LevelEditorSceneRenderer::Prepare()
	{
		SceneRenderer::Prepare();

		// Clear entity ID buffer to -1
		GetFrameBuffer()->ClearColorAttachment(1, { -1.0f, 0.0f, 0.0f, 0.0f });
	}

	void LevelEditorSceneRenderer::OnRenderScene()
	{
		switch (m_LevelEditor->GetSceneState())
		{
			case SceneState::Edit:
				BeginScene(*m_LevelEditor->GetEditorCamera());
				GetRenderSystem()->OnRenderEditor();
				break;
			case SceneState::Play:
			case SceneState::Pause:
				auto [camera, transform] = GetRenderSystem()->GetActiveCamera();
				if (!camera) return;

				BeginScene(*camera, transform);
				GetRenderSystem()->OnRenderRuntime();
				break;
		}
		EndScene();
	}

}
