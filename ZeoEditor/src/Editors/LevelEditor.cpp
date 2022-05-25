#include "Editors/LevelEditor.h"

#include "EditorUIRenderers/LevelEditorUIRenderer.h"
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
	}

	Ref<EditorUIRendererBase> LevelEditor::CreateEditorUIRenderer()
	{
		return CreateRef<LevelEditorUIRenderer>(SharedFromBase<LevelEditor>());
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

	AssetTypeId LevelEditor::GetAssetTypeId() const
	{
		return Level::TypeId();
	}

	void LevelEditor::LoadAsset(const std::string& path)
	{
		// Stop current playing scene
		if (m_SceneState != SceneState::Edit)
		{
			OnSceneStop();
		}
		m_LevelAsset = LevelLibrary::Get().LoadAsset(path);
		NewScene(false);
		m_LevelAsset->UpdateScene(GetScene());
	}

	void LevelEditor::LoadAndApplyDefaultAsset()
	{
		m_LevelAsset = LevelLibrary::GetDefaultEmptyLevel();
		m_LevelAsset->UpdateScene(GetScene());
	}

	void LevelEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
