#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class Level;
	class Scene;

	enum class SceneState
	{
		Edit = 0,
		Play = 1, Pause = 2,
	};

	class LevelEditor : public EditorBase
	{
		friend class LevelEditorViewPanel;

	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

		SceneState GetSceneState() const { return m_SceneState; }

	private:
		virtual Scope<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<Scene> CreateScene() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneResume();

		void OnDuplicateEntity();
		void OnDeleteEntity();

	public:
		virtual Ref<IAsset> GetAsset() const override { return m_LevelAsset; }
	private:
		virtual void LoadAsset(const std::filesystem::path& path) override;
		virtual void LoadAndApplyDefaultAsset() override;

		void ClearSelectedEntity();

	private:
		SceneState m_SceneState = SceneState::Edit;

		Ref<Scene> m_SceneForEdit;

		Ref<Level> m_LevelAsset;
	};

}
