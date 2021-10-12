#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class RenderSystem;
	class NativeScriptSystem;
	class PhysicsSystem;
	class LevelEditor;

	class LevelEditorScene : public Scene
	{
	public:
		explicit LevelEditorScene(const Ref<LevelEditor>& sceneEditor);
		virtual ~LevelEditorScene();

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;
		virtual void OnEvent(Event& e) override;

		void OnRuntimeStart();
		void OnRuntimeStop();

		virtual void PostLoad() override;

	private:
		void OnUpdateEditor(DeltaTime dt);
		void OnUpdateRuntime(DeltaTime dt);
		void OnRenderEditor(const EditorCamera& camera);
		void OnRenderRuntime();

	private:
		Scope<RenderSystem> m_RenderSystem;
		Scope<NativeScriptSystem> m_NativeScriptSystem;
		Scope<PhysicsSystem> m_PhysicsSystem;

		Ref<LevelEditor> m_SceneEditor;
	};

}
