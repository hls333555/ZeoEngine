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

	void AssetPreviewSceneRendererBase::RenderScene()
	{
		BeginScene(*m_EditorCamera);
		RenderEditor(true);
		EndScene();
	}

	void MaterialPreviewSceneRenderer::OnAttach(WorldBase* world)
	{
		AssetPreviewSceneRendererBase::OnAttach(world);

		RegisterRenderSystem<DirectionalLightRenderSystem>(world);
		RegisterRenderSystem<MeshRenderSystem>(world);
	}

	void MeshPreviewSceneRenderer::OnAttach(WorldBase* world)
	{
		AssetPreviewSceneRendererBase::OnAttach(world);

		RegisterRenderSystem<DirectionalLightRenderSystem>(world);
		RegisterRenderSystem<MeshPreviewRenderSystem>(world);
	}

	void ParticlePreviewSceneRenderer::OnAttach(WorldBase* world)
	{
		AssetPreviewSceneRendererBase::OnAttach(world);

		RegisterRenderSystem<ParticlePreviewRenderSystem>(world);
	}

}
