#include "Editors/MainEditor.h"

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	void MainEditor::OnAttach()
	{
		EditorBase::OnAttach();

		m_PreSceneCreate.connect<&MainEditor::ClearSelectedEntity>(this);
	}

	std::string MainEditor::GetAssetPath() const
	{
		return m_SceneAsset ? m_SceneAsset->GetPath() : "";
	}

	void MainEditor::PostSceneCreate(bool bIsFromLoad)
	{
		if (m_SceneAsset)
		{
			m_SceneAsset->ClearScene();
			m_SceneAsset = {};
		}
	}

	void MainEditor::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
	}

	void MainEditor::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
	}

	void MainEditor::OnScenePause()
	{
		m_SceneState = SceneState::Pause;
	}

	void MainEditor::OnSceneResume()
	{
		m_SceneState = SceneState::Play;
	}

	AssetTypeId MainEditor::GetAssetTypeId() const
	{
		return SceneAsset::TypeId();
	}

	void MainEditor::LoadAssetImpl(const std::string& path)
	{
		// Stop current playing scene
		OnSceneStop();
		m_SceneAsset = SceneAssetLibrary::Get().LoadAsset(path);
		m_SceneAsset->UpdateScene(GetScene());
	}

	void MainEditor::SaveAssetImpl(const std::string& path)
	{
		m_SceneAsset->Serialize(path);
	}

	void MainEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
