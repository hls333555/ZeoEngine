#include "SceneRenderers/LevelPreviewSceneRenderer.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/RenderSystems.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Worlds/LevelPreviewWorld.h"

namespace ZeoEngine {

	void LevelPreviewSceneRenderer::OnAttach(WorldBase* world)
	{
		SceneRenderer::OnAttach(world);

		m_LevelWorld = dynamic_cast<LevelPreviewWorld*>(world);
	}

	Scope<RenderGraph> LevelPreviewSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<ForwardRenderGraph>();
	}

	Scope<RenderSystemBase> LevelPreviewSceneRenderer::CreateRenderSystem(WorldBase* world)
	{
		return CreateScope<RenderSystem>(world);
	}

	void LevelPreviewSceneRenderer::OnRenderScene()
	{
		if (m_LevelWorld->IsRuntime())
		{
			const Entity cameraEntity = m_LevelWorld->GetActiveScene()->GetMainCameraEntity();
			if (!cameraEntity || m_LevelWorld->IsSimulation())
			{
				BeginScene(m_LevelWorld->GetEditorCamera());
			}
			else
			{
				const SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
				const Mat4& cameraTransform = cameraEntity.GetTransform();
				BeginScene(camera, cameraTransform);
			}
			GetRenderSystem()->OnRenderRuntime();
			GetRenderGraph().ToggleRenderPassActive("Grid", false);
			EndScene();
		}
		else
		{
			BeginScene(m_LevelWorld->GetEditorCamera());
			GetRenderSystem()->OnRenderEditor();
			GetRenderGraph().ToggleRenderPassActive("Grid", true);
			EndScene();
		}
	}

}
