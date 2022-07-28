#include "SceneRenderers/LevelEditorSceneRenderer.h"

#include "Editors/LevelEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/GameFramework/SceneCamera.h"

namespace ZeoEngine {

	LevelEditorSceneRenderer::LevelEditorSceneRenderer(const Ref<LevelEditor>& levelEditor)
		: m_LevelEditor(levelEditor)
	{
	}

	Scope<RenderGraph> LevelEditorSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<ForwardRenderGraph>();
	}

	Scope<RenderSystemBase> LevelEditorSceneRenderer::CreateRenderSystem(const Ref<Scene>& scene)
	{
		return CreateScope<RenderSystem>(scene, shared_from_this());
	}

	void LevelEditorSceneRenderer::OnRenderScene()
	{
		switch (m_LevelEditor->GetSceneState())
		{
			case SceneState::Edit:
				BeginScene(*m_LevelEditor->GetEditorCamera());
				GetRenderGraph().ToggleRenderPassActive("Grid", true);
				GetRenderSystem()->OnRenderEditor();
				break;
			case SceneState::Play:
			case SceneState::Pause:
				auto [sceneCamera, transform] = GetRenderSystem()->GetActiveSceneCamera();
				if (sceneCamera)
				{
					BeginScene(*sceneCamera, transform);
				}
				else
				{
					BeginScene(*m_LevelEditor->GetEditorCamera());
				}
				GetRenderGraph().ToggleRenderPassActive("Grid", false);
				GetRenderSystem()->OnRenderRuntime();
				break;
		}
		EndScene();
	}

}
