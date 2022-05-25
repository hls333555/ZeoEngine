#include "SceneRenderers/MaterialEditorSceneRenderer.h"

#include "Editors/MaterialEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	MaterialEditorSceneRenderer::MaterialEditorSceneRenderer(const Ref<MaterialEditor>& materialEditor)
		: m_MaterialEditor(materialEditor)
	{
	}

	Scope<RenderGraph> MaterialEditorSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<EditorPreviewRenderGraph>();
	}

	Scope<RenderSystemBase> MaterialEditorSceneRenderer::CreateRenderSystem(const Ref<Scene>& scene)
	{
		return CreateScope<MaterialPreviewRenderSystem>(scene, shared_from_this());
	}

	void MaterialEditorSceneRenderer::OnRenderScene()
	{
		BeginScene(*m_MaterialEditor->GetEditorCamera());
		GetRenderSystem()->OnRenderEditor();
		EndScene();
	}

}
