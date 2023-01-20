#include "Worlds/LevelPreviewWorld.h"

#include "Core/Editor.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Core/Input.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"
#include "Inspectors/EntityInspector.h"
#include "Panels/LevelViewPanel.h"
#include "SceneRenderers/LevelPreviewSceneRenderer.h"

namespace ZeoEngine {

	class LevelViewCameraSystem : public CameraSystem
	{
	public:
		using CameraSystem::CameraSystem;
	private:
		virtual void OnCameraComponentAdded(Scene& scene, entt::entity e) const override;
	};

	void LevelViewCameraSystem::OnCameraComponentAdded(Scene& scene, entt::entity e) const
	{
		CameraSystem::OnCameraComponentAdded(scene, e);

		// Level View Panel is not created yet when the very first scene creates
		if (const auto* levelView = g_Editor->GetPanel<LevelViewPanel>(LEVEL_VIEW))
		{
			const Entity entity{ e, scene.shared_from_this() };
			levelView->UpdateViewportSizeOnSceneCamera(entity.GetComponent<CameraComponent>());
		}
	}

	void LevelPreviewWorld::OnAttach()
	{
		EditorPreviewWorldBase::OnAttach();

		RegisterSystem<CommandSystem>(this);
		RegisterSystem<VisibilitySystem>(this);
		RegisterSystem<BoundsSystem>(this);
		RegisterSystem<LevelViewCameraSystem>(this);
		RegisterSystem<MeshSystem>(this);
		RegisterSystem<DirectionalLightSystem>(this);
		RegisterSystem<PointLightSystem>(this);
		RegisterSystem<SpotLightSystem>(this);
		RegisterSystem<ParticleUpdateSystem>(this);
		RegisterSystem<ScriptSystem>(this);
		RegisterSystem<TransformSystem>(this);
		RegisterSystem<PrePhysicsScriptSystem>(this);
		RegisterSystem<PhysicsSystem>(this);
		RegisterSystem<PostPhysicsTransformSystem>(this);
		RegisterSystem<PostPhysicsScriptSystem>(this);

		m_OnContextEntityChanged.connect<&LevelPreviewWorld::ActivateEntityInspector>(this);
	}

	void LevelPreviewWorld::OnUpdate(DeltaTime dt)
	{
		EditorPreviewWorldBase::OnUpdate(dt);

		if (m_SceneState == SceneState::Pause)
		{
			--m_StepFrames;
		}
	}

	void LevelPreviewWorld::StopScene()
	{
		if (IsRuntime())
		{
			OnSceneStop();
		}
	}

	void LevelPreviewWorld::PostSceneCreate()
	{
		m_SceneForEdit = GetActiveScene();
		// Clear last scene's selected entity so that component inspector will not reference an invalid entity
		SetContextEntity({});
	}

	Ref<SceneRenderer> LevelPreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<LevelPreviewSceneRenderer>();
	}

	Ref<IAsset> LevelPreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		StopScene();
		NewScene();
		Scene& scene = *GetActiveScene();
		return AssetLibrary::LoadAsset<Level>(path, bForce, &scene);
	}

	Scope<InspectorBase> LevelPreviewWorld::CreateInspector()
	{
		return CreateScope<EntityInspector>(this);
	}

	void LevelPreviewWorld::ActivateEntityInspector(Entity entity, Entity lastEntity)
	{
		if (entity)
		{
			g_Editor->InspectLevelEntity();
		}
	}

	void LevelPreviewWorld::OnScenePlay()
	{
		m_SceneState = SceneState::Run;
		m_bIsSimulation = false;
		TogglePlayMode(true);
		auto sceneForPlay = m_SceneForEdit->Copy();
		SetContextEntity({});
		SetActiveScene(std::move(sceneForPlay));
		OnPlayStart();
	}

	void LevelPreviewWorld::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		TogglePlayMode(false);
		SetContextEntity({});
		SetActiveScene(m_SceneForEdit);
		OnPlayStop();
	}

	void LevelPreviewWorld::OnSceneStartSimulation()
	{
		m_SceneState = SceneState::Run;
		m_bIsSimulation = true;
		auto sceneForSimulate = m_SceneForEdit->Copy();
		SetContextEntity({});
		SetActiveScene(std::move(sceneForSimulate));
		OnSimulationStart();
	}

	void LevelPreviewWorld::OnSceneStopSimulation()
	{
		m_SceneState = SceneState::Edit;
		SetContextEntity({});
		SetActiveScene(m_SceneForEdit);
		OnSimulationStop();
	}

	void LevelPreviewWorld::OnScenePause()
	{
		m_SceneState = SceneState::Pause;
		if (!m_bIsSimulation)
		{
			TogglePlayMode(false);
		}
	}

	void LevelPreviewWorld::OnSceneResume()
	{
		m_SceneState = SceneState::Run;
		if (!m_bIsSimulation)
		{
			TogglePlayMode(true);
			// TODO: We may want to hide gizmo but continue show entity inspector in some way
			SetContextEntity({});
		}
	}

	void LevelPreviewWorld::OnDuplicateEntity()
	{
		if (IsRuntime()) return;

		const Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			const Entity newEntity = GetActiveScene()->DuplicateEntity(selectedEntity);
			SetContextEntity(newEntity);
		}
	}

	void LevelPreviewWorld::OnDeleteEntity()
	{
		if (IsRuntime()) return;

		const Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			GetActiveScene()->DestroyEntity(selectedEntity);
			SetContextEntity({});
		}
	}

	void LevelPreviewWorld::OnPlayStart() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnPlayStart();
		}
	}

	void LevelPreviewWorld::OnPlayStop() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnPlayStop();
		}
	}

	void LevelPreviewWorld::OnSimulationStart() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnSimulationStart();
		}
	}

	void LevelPreviewWorld::OnSimulationStop() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnSimulationStop();
		}
	}

	void LevelPreviewWorld::TogglePlayMode(bool bEnable) const
	{
		Input::SetCursorMode(bEnable ? CursorMode::Locked : CursorMode::Normal);
		ImGui::SetMouseEnabled(!bEnable);
		ImGui::SetKeyboardNavEnabled(!bEnable);
	}

}
