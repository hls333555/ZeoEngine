#include "SceneRenderers/AssetPreviewSceneRenderers.h"

#include "Worlds/EditorPreviewWorldBase.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	void AssetPreviewSceneRendererBase::OnAttach(const Ref<WorldBase>& world)
	{
		SceneRenderer::OnAttach(world);

		m_EditorCamera = &std::dynamic_pointer_cast<EditorPreviewWorldBase>(world)->GetEditorCamera();
	}

	Scope<RenderGraph> AssetPreviewSceneRendererBase::CreateRenderGraph()
	{
		return CreateScope<AssetPreviewRenderGraph>();
	}

	void AssetPreviewSceneRendererBase::OnRenderScene()
	{
		BeginScene(*m_EditorCamera);
		GetRenderSystem()->OnRenderEditor();
		EndScene();
	}

	Scope<RenderSystemBase> MaterialPreviewSceneRenderer::CreateRenderSystem(const Ref<WorldBase>& world)
	{
		return CreateScope<MaterialPreviewRenderSystem>(world);
	}

	Scope<RenderSystemBase> MeshPreviewSceneRenderer::CreateRenderSystem(const Ref<WorldBase>& world)
	{
		return CreateScope<MeshPreviewRenderSystem>(world);
	}

	Scope<RenderSystemBase> ParticlePreviewSceneRenderer::CreateRenderSystem(const Ref<WorldBase>& world)
	{
		return CreateScope<ParticlePreviewRenderSystem>(world);
	}

}
