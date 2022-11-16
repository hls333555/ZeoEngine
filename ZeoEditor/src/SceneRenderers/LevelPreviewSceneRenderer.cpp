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

		RegisterRenderSystem<BillboardRenderSystem>(world);
		RegisterRenderSystem<CameraVisualizerRenderSystem>(world);
		RegisterRenderSystem<PhysicsDebugRenderSystem>(world);
		RegisterRenderSystem<MeshRenderSystem>(world);
		RegisterRenderSystem<DirectionalLightRenderSystem>(world);
		RegisterRenderSystem<PointLightRenderSystem>(world);
		RegisterRenderSystem<SpotLightRenderSystem>(world);

		m_LevelWorld = dynamic_cast<LevelPreviewWorld*>(world);
	}

	Scope<RenderGraph> LevelPreviewSceneRenderer::CreateRenderGraph()
	{
		return CreateScope<ForwardRenderGraph>();
	}

	void LevelPreviewSceneRenderer::RenderScene()
	{
		if (m_LevelWorld->IsRuntime())
		{
			GetRenderGraph().ToggleRenderPassActive("Grid", false);
			if (m_LevelWorld->IsSimulation())
			{
				BeginScene(m_LevelWorld->GetEditorCamera());
			}
			else
			{
				const Entity cameraEntity = m_LevelWorld->GetActiveScene()->GetMainCameraEntity();
				if (!cameraEntity)
				{
					EndScene();
					ZE_CORE_ERROR("No active camera for rendering!");
					return;
				}

				const SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
				const Mat4& cameraTransform = cameraEntity.GetTransform();
				BeginScene(camera, cameraTransform);
			}
			RenderRuntime();
			EndScene();
		}
		else
		{
			BeginScene(m_LevelWorld->GetEditorCamera());
			RenderEditor(false);
			GetRenderGraph().ToggleRenderPassActive("Grid", true);
			EndScene();
		}
	}

}
