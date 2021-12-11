#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class SceneAsset;
	class LevelEditorScene;

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
		virtual Ref<EditorUIRendererBase> CreateEditorUIRenderer() override;
		virtual Ref<Scene> CreateScene() override;

		void UpdateSceneRef(bool bIsFromLoad);

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneResume();
		void OnDuplicateEntity();
		void OnDeleteEntity();

	public:
		virtual AssetHandle<IAsset> GetAsset() const override { return m_SceneAsset; }
		virtual AssetTypeId GetAssetTypeId() const override;
	private:
		virtual void LoadAsset(const std::string& path) override;
		virtual void SaveAsset(const std::string& path) override;

		virtual Ref<FrameBuffer> CreateFrameBuffer() override;
		virtual Scope<RenderGraph> CreateRenderGraph(const Ref<FrameBuffer>& fbo) override;

		void ClearSelectedEntity();

	private:
		SceneState m_SceneState = SceneState::Edit;

		Ref<LevelEditorScene> m_SceneForEdit;

		AssetHandle<SceneAsset> m_SceneAsset;
	};

}
