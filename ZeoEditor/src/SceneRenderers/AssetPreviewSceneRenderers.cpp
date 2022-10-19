#include "SceneRenderers/AssetPreviewSceneRenderers.h"

#include "Worlds/EditorPreviewWorldBase.h"
#include "Engine/GameFramework/RenderSystems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	void AssetPreviewSceneRendererBase::OnAttach(WorldBase* world)
	{
		SceneRenderer::OnAttach(world);

		m_EditorCamera = &dynamic_cast<EditorPreviewWorldBase*>(world)->GetEditorCamera();
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

	Scope<RenderSystemBase> MaterialPreviewSceneRenderer::CreateRenderSystem(WorldBase* world)
	{
		return CreateScope<MaterialPreviewRenderSystem>(world);
	}

	Scope<RenderSystemBase> MeshPreviewSceneRenderer::CreateRenderSystem(WorldBase* world)
	{
		return CreateScope<MeshPreviewRenderSystem>(world);
	}

	Scope<RenderSystemBase> ParticlePreviewSceneRenderer::CreateRenderSystem(WorldBase* world)
	{
		return CreateScope<ParticlePreviewRenderSystem>(world);
	}

}
