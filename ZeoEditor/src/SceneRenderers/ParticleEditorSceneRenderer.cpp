#include "SceneRenderers/ParticleEditorSceneRenderer.h"

#include "Editors/ParticleEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	ParticleEditorSceneRenderer::ParticleEditorSceneRenderer(const Ref<ParticleEditor>& particleEditor)
		: m_ParticleEditor(particleEditor)
	{
	}

	Scope<RenderGraph> ParticleEditorSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<EditorPreviewRenderGraph>();
	}

	Scope<RenderSystemBase> ParticleEditorSceneRenderer::CreateRenderSystem(const Ref<Scene>& scene)
	{
		return CreateScope<ParticlePreviewRenderSystem>(scene, shared_from_this());
	}

	void ParticleEditorSceneRenderer::OnRenderScene()
	{
		BeginScene(*m_ParticleEditor->GetEditorCamera());
		GetRenderSystem()->OnRenderEditor();
		EndScene();
	}

}
