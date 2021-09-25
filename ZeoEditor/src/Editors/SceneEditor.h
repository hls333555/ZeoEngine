#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class SceneAsset;

	enum class SceneState
	{
		Edit = 0,
		Play = 1, Pause = 2,
	};

	class SceneEditor : public EditorBase
	{
		friend class SceneViewPanel;

	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

		SceneState GetSceneState() const { return m_SceneState; }

	private:
		virtual Ref<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<Scene> CreateScene() override;

		void UpdateSceneRef(bool bIsFromLoad);

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneResume();

		virtual AssetTypeId GetAssetTypeId() const override;
		virtual AssetHandle<IAsset> GetAsset() const override { return m_SceneAsset; }
		virtual void LoadAsset(const std::string& path) override;
		virtual void SaveAsset(const std::string& path) override;

		void ClearSelectedEntity();

	private:
		SceneState m_SceneState = SceneState::Edit;

		AssetHandle<SceneAsset> m_SceneAsset;
	};

}
