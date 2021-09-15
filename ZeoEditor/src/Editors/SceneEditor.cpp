#include "Editors/SceneEditor.h"

#include "Engine/GameFramework/Scene.h"

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
	}

	void SceneEditor::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
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
