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

	class LevelPreviewObserverSystem : public LevelObserverSystem
	{
	public:
		virtual void OnBind() override;
	};

	void LevelPreviewObserverSystem::OnBind()
	{
		LevelObserverSystem::OnBind();

		// Level View Panel is not created yet when the very first scene creates
		if (const auto* levelView = g_Editor->GetPanel<LevelViewPanel>(LEVEL_VIEW))
		{
			BindOnComponentAdded<CameraComponent, &ViewPanelBase::OnCameraComponentAdded>(*levelView);
		}
	}

	void LevelPreviewWorld::OnAttach()
	{
		EditorPreviewWorldBase::OnAttach();

		RegisterSystem<ParticleUpdateSystem>(this);
		RegisterSystem<PhysicsSystem>(this);
		RegisterSystem<CommandSystem>(this);
		RegisterSystem<ScriptSystem>(this);

		m_OnContextEntityChanged.connect<&LevelPreviewWorld::ActivateEntityInspector>(this);
	}

	void LevelPreviewWorld::StopScene()
	{
		if (IsRuntime())
		{
			OnSceneStop();
		}
	}

	void LevelPreviewWorld::PostSceneCreate(const Ref<Scene>& scene)
	{
		m_SceneForEdit = scene;
		SetContextEntity({});
	}

	Ref<Scene> LevelPreviewWorld::CreateScene()
	{
		SceneSpec spec;
		spec.SceneObserverSystem = CreateScope<LevelPreviewObserverSystem>();
		spec.bIsPhysicalScene = true;
		return CreateRef<Scene>(std::move(spec));
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
		Input::SetCursorMode(CursorMode::Locked);
		ImGui::SetMouseEnabled(false);
		ImGui::SetKeyboardNavEnabled(false);
		auto sceneForPlay = m_SceneForEdit->Copy();
		SetContextEntity({});
		SetActiveScene(std::move(sceneForPlay));
		OnPlayStart();
	}

	void LevelPreviewWorld::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		Input::SetCursorMode(CursorMode::Normal);
		ImGui::SetMouseEnabled(true);
		ImGui::SetKeyboardNavEnabled(true);
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
	}

	void LevelPreviewWorld::OnSceneResume()
	{
		m_SceneState = SceneState::Run;
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

}
