#pragma once

#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	enum class SceneState
	{
		Edit = 0,
		Run = 1, // Can be play or simulate
		Pause = 2 // Can be play or simulate
	};

	class LevelPreviewWorld : public EditorPreviewWorldBase
	{
	public:
		using EditorPreviewWorldBase::EditorPreviewWorldBase;

		virtual void OnAttach() override;

		virtual bool IsRuntime() const override { return m_SceneState != SceneState::Edit; }
		/** Returns true if current runtime is simulation. Calling it when SceneState == Edit is meaningless. */
		virtual bool IsSimulation() const override { return m_bIsSimulation; }

		void StopScene();

		SceneState GetSceneState() const { return m_SceneState; }

		void OnScenePlay();
		void OnSceneStop();
		void OnSceneStartSimulation();
		void OnSceneStopSimulation();
		void OnScenePause();
		void OnSceneResume();

		void OnSceneStep(I32 frames = 1) { m_StepFrames = frames; }

		void OnDuplicateEntity();
		void OnDeleteEntity();

	private:
		virtual void PostSceneCreate() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;

		void ActivateEntityInspector(Entity entity, Entity lastEntity);

		void OnPlayStart() const;
		void OnPlayStop() const;
		void OnSimulationStart() const;
		void OnSimulationStop() const;

		virtual bool IsRunning() override { return m_SceneState == SceneState::Run || (m_SceneState == SceneState::Pause && m_StepFrames-- > 0); }

	private:
		SceneState m_SceneState = SceneState::Edit;
		bool m_bIsSimulation = false;

		Ref<Scene> m_SceneForEdit;

		I32 m_StepFrames = 0;
	};
	
}
