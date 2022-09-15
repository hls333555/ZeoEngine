#include "SceneRenderers/LevelPreviewSceneRenderer.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Worlds/LevelPreviewWorld.h"

namespace ZeoEngine {

	void LevelPreviewSceneRenderer::OnAttach(const Ref<WorldBase>& world)
	{
		SceneRenderer::OnAttach(world);

		m_LevelWorld = std::dynamic_pointer_cast<LevelPreviewWorld>(world);
	}

	Scope<RenderGraph> LevelPreviewSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<ForwardRenderGraph>();
	}

	Scope<RenderSystemBase> LevelPreviewSceneRenderer::CreateRenderSystem(const Ref<WorldBase>& world)
	{
		return CreateScope<RenderSystem>(world);
	}

	void LevelPreviewSceneRenderer::OnRenderScene()
	{
		const auto levelWorld = m_LevelWorld.lock();
		switch (levelWorld->GetSceneState())
		{
			case SceneState::Edit:
				BeginScene(levelWorld->GetEditorCamera());
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
					BeginScene(levelWorld->GetEditorCamera());
				}
				GetRenderGraph().ToggleRenderPassActive("Grid", false);
				GetRenderSystem()->OnRenderRuntime();
				break;
		}
		EndScene();
	}

}
