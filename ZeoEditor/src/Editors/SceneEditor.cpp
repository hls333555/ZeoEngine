#include "Editors/SceneEditor.h"

#include "EditorUIRenderers/SceneEditorUIRenderer.h"
#include "Scenes/EditorScene.h"

namespace ZeoEngine {

	void SceneEditor::OnAttach()
	{
		m_SceneAsset = SceneAsset::Create();

		// Bind delegates before scene creation
		m_PreSceneCreate.connect<&SceneEditor::ClearSelectedEntity>(this);
		m_PostSceneCreate.connect<&SceneEditor::UpdateSceneRef>(this);

		EditorBase::OnAttach();
	}

	Ref<EditorUIRendererBase> SceneEditor::CreateEditorUIRenderer()
	{
		return CreateRef<SceneEditorUIRenderer>(SharedFromBase<SceneEditor>());
	}

	Ref<Scene> SceneEditor::CreateScene()
	{
		m_SceneForEdit = CreateRef<EditorScene>(SharedFromBase<SceneEditor>());
		return m_SceneForEdit;
	}

	void SceneEditor::UpdateSceneRef(bool bIsFromLoad)
	{
		if (!bIsFromLoad)
		{
			m_SceneAsset->UpdateScene(GetScene());
		}
	}

	void SceneEditor::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		auto sceneForPlay = m_SceneForEdit->Copy<EditorScene>(SharedFromBase<SceneEditor>());
		SetActiveScene(sceneForPlay);
		SetContextEntity({});
		GetScene<EditorScene>()->OnRuntimeStart();
	}

	void SceneEditor::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		GetScene<EditorScene>()->OnRuntimeStop();
		SetActiveScene(m_SceneForEdit);
	}

	void SceneEditor::OnScenePause()
	{
		m_SceneState = SceneState::Pause;
	}

	void SceneEditor::OnSceneResume()
	{
		m_SceneState = SceneState::Play;
	}

	void SceneEditor::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit) return;

		Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			Entity newEntity = GetScene()->DuplicateEntity(selectedEntity);
			SetContextEntity(newEntity);
		}
	}

	void SceneEditor::OnDeleteEntity()
	{
		if (m_SceneState != SceneState::Edit) return;

		Entity selectedEntity = GetContextEntity();
		if (selectedEntity)
		{
			GetScene()->DestroyEntity(selectedEntity);
			SetContextEntity({});
		}
	}

	AssetTypeId SceneEditor::GetAssetTypeId() const
	{
		return SceneAsset::TypeId();
	}

	void SceneEditor::LoadAsset(const std::string& path)
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

	void SceneEditor::SaveAsset(const std::string& path)
	{
		m_SceneAsset->Serialize(path);
	}

	void SceneEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
