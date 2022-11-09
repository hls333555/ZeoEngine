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

		virtual bool IsRuntime() const override { return m_SceneState != SceneState::Edit; }

		void StopScene();

		SceneState GetSceneState() const { return m_SceneState; }

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneResume();

		void OnSceneStep(I32 frames = 1) { m_StepFrames = frames; }

		void OnDuplicateEntity();
		void OnDeleteEntity();

	private:
		virtual Scope<SceneObserverSystemBase> CreateSceneObserverSystem() override;
		virtual void PostSceneCreate(const Ref<Scene>& scene) override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;

		void ActivateEntityInspector(Entity entity, Entity lastEntity);

		void OnRuntimeStart() const;
		void OnRuntimeStop() const;

		virtual bool IsRunning() override { return m_SceneState == SceneState::Play || (m_SceneState == SceneState::Pause && m_StepFrames-- > 0); }

	private:
		SceneState m_SceneState = SceneState::Edit;

		Ref<Scene> m_SceneForEdit;

		I32 m_StepFrames = 0;
	};
	
}