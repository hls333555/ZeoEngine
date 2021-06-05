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

	AssetTypeId MainEditor::GetAssetTypeId() const
	{
		return SceneAsset::TypeId();
	}

	void MainEditor::LoadAssetImpl(const std::string& path)
	{
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
