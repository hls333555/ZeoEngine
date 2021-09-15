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

		virtual std::string GetAssetPath() const override;

		SceneState GetSceneState() const { return m_SceneState; }

	private:
		virtual void PostSceneCreate(bool bIsFromLoad) override;

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneResume();

		virtual AssetTypeId GetAssetTypeId() const override;
		virtual void LoadAssetImpl(const std::string& path) override;
		virtual void SaveAssetImpl(const std::string& path) override;

		void ClearSelectedEntity();

	private:
		AssetHandle<SceneAsset> m_SceneAsset;
		SceneState m_SceneState = SceneState::Edit;
	};

}
