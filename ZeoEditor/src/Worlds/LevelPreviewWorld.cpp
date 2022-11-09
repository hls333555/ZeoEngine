#include "Worlds/LevelPreviewWorld.h"

#include "Core/Editor.h"
#include "Engine/Asset/AssetLibrary.h"
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

		const auto* levelView = g_Editor->GetPanel<LevelViewPanel>(LEVEL_VIEW);
		BindOnComponentAdded<CameraComponent, &ViewPanelBase::OnCameraComponentAdded>(*levelView);
	}

	void LevelPreviewWorld::OnAttach()
	{
		// Register systems before scene construction to receive the first OnActiveSceneChanged delegate call
		RegisterSystem<ParticleUpdateSystem>(this);
		RegisterSystem<PhysicsSystem>(this);

		RegisterSystem<ScriptSystem>(this);

		EditorPreviewWorldBase::OnAttach();

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
		m_SceneState = SceneState::Play;
		auto sceneForPlay = m_SceneForEdit->Copy();
		SetContextEntity({});
		SetActiveScene(std::move(sceneForPlay));
		OnRuntimeStart();
		GetEditorCamera().SetEnableUpdate(false);
	}

	void LevelPreviewWorld::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		SetContextEntity({});
		SetActiveScene(m_SceneForEdit);
		OnRuntimeStop();
		GetEditorCamera().SetEnableUpdate(true);
	}

	void LevelPreviewWorld::OnScenePause()
	{
		m_SceneState = SceneState::Pause;
	}

	void LevelPreviewWorld::OnSceneResume()
	{
		m_SceneState = SceneState::Play;
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

	void LevelPreviewWorld::OnRuntimeStart() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnRuntimeStart();
		}
	}

	void LevelPreviewWorld::OnRuntimeStop() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnRuntimeStop();
		}
	}

}
