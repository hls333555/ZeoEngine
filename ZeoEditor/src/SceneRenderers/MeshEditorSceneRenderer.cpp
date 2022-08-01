#include "SceneRenderers/MeshEditorSceneRenderer.h"

#include "Editors/MeshEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	MeshEditorSceneRenderer::MeshEditorSceneRenderer(const Ref<MeshEditor>& meshEditor)
		: m_MeshEditor(meshEditor)
	{
	}

	Scope<RenderGraph> MeshEditorSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<EditorPreviewRenderGraph>();
	}

	Scope<RenderSystemBase> MeshEditorSceneRenderer::CreateRenderSystem(const Ref<Scene>& scene)
	{
		return CreateScope<MeshPreviewRenderSystem>(scene, shared_from_this());
	}

	void MeshEditorSceneRenderer::OnRenderScene()
	{
		BeginScene(*m_MeshEditor->GetEditorCamera());
		GetRenderSystem()->OnRenderEditor();
		EndScene();
	}

}
