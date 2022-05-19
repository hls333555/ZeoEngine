#include "SceneRenderers/LevelEditorSceneRenderer.h"

#include "Editors/LevelEditor.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	LevelEditorSceneRenderer::LevelEditorSceneRenderer(const Ref<LevelEditor>& levelEditor)
		: m_LevelEditor(levelEditor)
	{
	}

	Scope<RenderGraph> LevelEditorSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<ForwardRenderGraph>();
	}

	Scope<RenderSystemBase> LevelEditorSceneRenderer::CreateRenderSystem()
	{
		return CreateScope<RenderSystem>(shared_from_this());
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
