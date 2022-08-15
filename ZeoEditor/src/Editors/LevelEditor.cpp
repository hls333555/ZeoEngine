#include "Editors/LevelEditor.h"

#include "EditorUIRenderers/LevelEditorUIRenderer.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Scenes/LevelEditorScene.h"
#include "SceneRenderers/LevelEditorSceneRenderer.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	void LevelEditor::OnAttach()
	{
		// Bind delegates before scene creation
		m_PreSceneCreate.connect<&LevelEditor::ClearSelectedEntity>(this);

		EditorBase::OnAttach();

		LoadAndApplyDefaultAsset();
	}

	Scope<EditorUIRendererBase> LevelEditor::CreateEditorUIRenderer()
	{
		return CreateScope<LevelEditorUIRenderer>(SharedFromBase<LevelEditor>());
	}

	Ref<Scene> LevelEditor::CreateScene()
	{
		m_SceneForEdit = CreateRef<LevelEditorScene>(SharedFromBase<LevelEditor>());
		return m_SceneForEdit;
	}

	Ref<SceneRenderer> LevelEditor::CreateSceneRenderer()
	{
		return CreateRef<LevelEditorSceneRenderer>(SharedFromBase<LevelEditor>());
	}

	void LevelEditor::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		const auto sceneForPlay = m_SceneForEdit->Copy<LevelEditorScene>(SharedFromBase<LevelEditor>());
		SetContextEntity({});
		for (const auto& system : GetScene()->GetSystems())
		{
			system->UpdateScene(sceneForPlay);
			system->BindUpdateFuncToRuntime();
		}
		SetActiveScene(sceneForPlay, false);
		GetScene<LevelEditorScene>()->OnRuntimeStart();
		GetEditorCamera()->SetEnableUpdate(false);
	}

	void LevelEditor::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		for (const auto& system : GetScene()->GetSystems())
		{
			system->UpdateScene(m_SceneForEdit);
			system->BindUpdateFuncToEditor();
		}
		SetContextEntity({});
		SetActiveScene(m_SceneForEdit, false);
		GetScene<LevelEditorScene>()->OnRuntimeStop();
		GetEditorCamera()->SetEnableUpdate(true);
	}

	void LevelEditor::OnScenePause()
	{
		m_SceneState = SceneState::Pause;
	}

	void LevelEditor::OnSceneResume()
	{
		m_SceneState = SceneState::Play;
	}

	void LevelEditor::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit) return;

		Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			Entity newEntity = GetScene()->DuplicateEntity(selectedEntity);
			SetContextEntity(newEntity);
		}
	}

	void LevelEditor::OnDeleteEntity()
	{
		if (m_SceneState != SceneState::Edit) return;

		Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			GetScene()->DestroyEntity(selectedEntity);
			SetContextEntity({});
		}
	}

	void LevelEditor::LoadAsset(const std::filesystem::path& path)
	{
		// Stop current playing scene
		if (m_SceneState != SceneState::Edit)
		{
			OnSceneStop();
		}
		NewScene(false);
		auto scene = GetScene();
		m_LevelAsset = AssetLibrary::LoadAsset<Level>(path, AssetLibrary::DeserializeMode::Force, &scene);
	}

	void LevelEditor::SaveAsset(const std::filesystem::path& path)
	{
		AssetManager::Get().SaveAsset(path, m_LevelAsset);
		// There are mainly four situations:
		// Save to current level asset, no need to deserialize at all
		// Save to a new level asset, we share current scene with new asset so we do not need to deserialize again
		// Save to an existing unloaded level asset, we share current scene with new asset so we do not need to deserialize again
		// Save to an existing loaded level asset, just update scene and it's done
		m_LevelAsset = AssetLibrary::LoadAsset<Level>(path, AssetLibrary::DeserializeMode::Ignore);
		m_LevelAsset->SetScene(GetScene());
	}

	void LevelEditor::LoadAndApplyDefaultAsset()
	{
		auto scene = GetScene();
		m_LevelAsset = AssetLibrary::LoadAsset<Level>(Level::GetTemplatePath(), AssetLibrary::DeserializeMode::Force, &scene);
	}

	void LevelEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
