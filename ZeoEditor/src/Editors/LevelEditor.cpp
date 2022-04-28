#include "Editors/LevelEditor.h"

#include "EditorUIRenderers/LevelEditorUIRenderer.h"
#include "Scenes/LevelEditorScene.h"
#include "SceneRenderers/LevelEditorSceneRenderer.h"
#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	void LevelEditor::OnAttach()
	{
		m_SceneAsset = SceneAsset::Create()->GetAssetHandle();

		// Bind delegates before scene creation
		m_PreSceneCreate.connect<&LevelEditor::ClearSelectedEntity>(this);
		m_PostSceneCreate.connect<&LevelEditor::UpdateSceneRef>(this);

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

	void LevelEditor::UpdateSceneRef(const Ref<Scene>& scene, bool bIsFromLoad)
	{
		if (!bIsFromLoad)
		{
			m_SceneAsset->UpdateScene(scene);
		}
	}

	void LevelEditor::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		auto sceneForPlay = m_SceneForEdit->Copy<LevelEditorScene>(SharedFromBase<LevelEditor>());
		SetActiveScene(sceneForPlay);
		SetContextEntity({});
		for (const auto& system : GetScene()->GetSystems())
		{
			system->UpdateScene(sceneForPlay);
			system->BindUpdateFuncToRuntime();
		}
		GetSceneRenderer()->GetRenderSystem()->UpdateScene(sceneForPlay);
		GetScene<LevelEditorScene>()->OnRuntimeStart();
	}

	void LevelEditor::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		for (const auto& system : GetScene()->GetSystems())
		{
			system->UpdateScene(m_SceneForEdit);
			system->BindUpdateFuncToEditor();
		}
		GetSceneRenderer()->GetRenderSystem()->UpdateScene(m_SceneForEdit);
		GetScene<LevelEditorScene>()->OnRuntimeStop();
		SetActiveScene(m_SceneForEdit);
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
		return SceneAsset::TypeId();
	}

	void LevelEditor::LoadAsset(const std::string& path)
	{
		// Stop current playing scene
		if (m_SceneState != SceneState::Edit)
		{
			OnSceneStop();
		}
		m_SceneAsset = SceneAssetLibrary::Get().LoadAsset(path);
		m_SceneAsset->UpdateScene(GetScene());
		m_SceneAsset->Deserialize();
	}

	void LevelEditor::SaveAsset(const std::string& path)
	{
		m_SceneAsset->Serialize(path);
	}

	void LevelEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
