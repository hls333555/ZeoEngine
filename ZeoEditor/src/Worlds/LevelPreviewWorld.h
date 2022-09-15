#pragma once

#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	enum class SceneState
	{
		Edit = 0,
		Play = 1, Pause = 2
	};

	class LevelPreviewWorld : public EditorPreviewWorldBase
	{
	public:
		using EditorPreviewWorldBase::EditorPreviewWorldBase;

		virtual void OnAttach() override;

		virtual void LoadAsset(const std::string& path) override;
		virtual void OnAssetSaveAs(const std::string& path) override;

		void StopScene();

		SceneState GetSceneState() const { return m_SceneState; }

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneResume();

		void OnDuplicateEntity();
		void OnDeleteEntity();

	private:
		virtual Ref<Scene> CreateScene() override;
		virtual void PostSceneCreate(const Ref<Scene>& scene) override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

		virtual Scope<InspectorBase> CreateInspector() override;

		void ActivateEntityInspector(Entity entity, Entity lastEntity);

	private:
		SceneState m_SceneState = SceneState::Edit;

		Ref<Scene> m_SceneForEdit;
	};
	
}
