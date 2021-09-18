#include "Editors/SceneEditor.h"

#include "EditorUIRenderers/SceneEditorUIRenderer.h"
#include "Scenes/EditorScene.h"

namespace ZeoEngine {

	void SceneEditor::OnAttach()
	{
		EditorBase::OnAttach();

		m_PreSceneCreate.connect<&SceneEditor::ClearSelectedEntity>(this);
	}

	std::string SceneEditor::GetAssetPath() const
	{
		return m_SceneAsset ? m_SceneAsset->GetPath() : "";
	}

	Ref<EditorUIRendererBase> SceneEditor::CreateEditorUIRenderer()
	{
		return CreateRef<SceneEditorUIRenderer>(SharedFromBase<SceneEditor>());
	}

	Ref<Scene> SceneEditor::CreateScene()
	{
		return CreateRef<EditorScene>(SharedFromBase<SceneEditor>());
	}

	void SceneEditor::PostSceneCreate(bool bIsFromLoad)
	{
		if (m_SceneAsset)
		{
			m_SceneAsset->ClearScene();
			m_SceneAsset = {};
		}
	}

	void SceneEditor::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		GetScene<EditorScene>()->OnRuntimeStart();
	}

	void SceneEditor::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		GetScene<EditorScene>()->OnRuntimeStop();
	}

	void SceneEditor::OnScenePause()
	{
		m_SceneState = SceneState::Pause;
	}

	void SceneEditor::OnSceneResume()
	{
		m_SceneState = SceneState::Play;
	}

	AssetTypeId SceneEditor::GetAssetTypeId() const
	{
		return SceneAsset::TypeId();
	}

	void SceneEditor::LoadAssetImpl(const std::string& path)
	{
		// Stop current playing scene
		OnSceneStop();
		m_SceneAsset = SceneAssetLibrary::Get().LoadAsset(path);
		m_SceneAsset->UpdateScene(GetScene());
	}

	void SceneEditor::SaveAssetImpl(const std::string& path)
	{
		m_SceneAsset->Serialize(path);
	}

	void SceneEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
