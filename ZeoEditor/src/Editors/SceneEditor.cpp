#include "Editors/SceneEditor.h"

#include "EditorUIRenderers/SceneEditorUIRenderer.h"
#include "Scenes/EditorScene.h"

namespace ZeoEngine {

	void SceneEditor::OnAttach()
	{
		m_SceneAsset = SceneAsset::Create("");
		// Bind delegates before scene creation
		m_PreSceneCreate.connect<&SceneEditor::ClearSelectedEntity>(this);
		m_PostSceneCreate.connect<&SceneEditor::UpdateSceneRef>(this);

		EditorBase::OnAttach();
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
		m_SceneAsset->Deserialize();
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
