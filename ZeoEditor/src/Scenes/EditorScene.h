#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class RenderSystem;
	class NativeScriptSystem;
	class PhysicsSystem;
	class SceneEditor;

	class EditorScene : public Scene
	{
	public:
		explicit EditorScene(const Ref<SceneEditor>& sceneEditor);
		virtual ~EditorScene();

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

		Ref<SceneEditor> m_SceneEditor;
	};

}
