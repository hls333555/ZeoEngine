#include "Worlds/LevelPreviewWorld.h"

#include "Core/Editor.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Systems.h"
#include "Inspectors/EntityInspector.h"
#include "Scenes/LevelPreviewScene.h"
#include "SceneRenderers/LevelPreviewSceneRenderer.h"

namespace ZeoEngine {

	void LevelPreviewWorld::OnAttach()
	{
		EditorPreviewWorldBase::OnAttach();

		m_OnContextEntityChanged.connect<&LevelPreviewWorld::ActivateEntityInspector>(this);
	}

	void LevelPreviewWorld::LoadAsset(const std::string& path)
	{
		StopScene();
		NewScene();
		// An empty scene is created every time, so we have to deserialize every time
		Ref<Level> level = AssetLibrary::LoadAsset<Level>(path, AssetLibrary::DeserializeMode::Force, &GetActiveScene());
		SetAsset(std::move(level));
		GetActiveScene()->PostLoad();
	}

	void LevelPreviewWorld::OnAssetSaveAs(const std::string& path)
	{
		LoadAsset(path);
	}

	void LevelPreviewWorld::StopScene()
	{
		if (m_SceneState != SceneState::Edit)
		{
			OnSceneStop();
		}
	}

	Ref<Scene> LevelPreviewWorld::CreateScene()
	{
		SetContextEntity({});
		m_SceneForEdit = CreateRef<LevelPreviewScene>();
		return m_SceneForEdit;
	}

	void LevelPreviewWorld::PostSceneCreate(const Ref<Scene>& scene)
	{
		SetContextEntity({});
	}

	Ref<SceneRenderer> LevelPreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<LevelPreviewSceneRenderer>();
	}

	Scope<InspectorBase> LevelPreviewWorld::CreateInspector()
	{
		return CreateScope<EntityInspector>(SharedFromBase<LevelPreviewWorld>());
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
		auto sceneForPlay = m_SceneForEdit->Copy<LevelPreviewScene>();
		SetContextEntity({});
		for (const auto& system : GetActiveScene()->GetSystems())
		{
			system->BindUpdateFuncToRuntime();
		}
		SetActiveScene(std::move(sceneForPlay));
		GetActiveScene<LevelPreviewScene>()->OnRuntimeStart();
		GetEditorCamera().SetEnableUpdate(false);
	}

	void LevelPreviewWorld::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		for (const auto& system : GetActiveScene()->GetSystems())
		{
			system->BindUpdateFuncToEditor();
		}
		SetContextEntity({});
		SetActiveScene(m_SceneForEdit);
		GetActiveScene<LevelPreviewScene>()->OnRuntimeStop();
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
		if (m_SceneState != SceneState::Edit) return;

		const Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			const Entity newEntity = GetActiveScene()->DuplicateEntity(selectedEntity);
			SetContextEntity(newEntity);
		}
	}

	void LevelPreviewWorld::OnDeleteEntity()
	{
		if (m_SceneState != SceneState::Edit) return;

		const Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			GetActiveScene()->DestroyEntity(selectedEntity);
			SetContextEntity({});
		}
	}

}
