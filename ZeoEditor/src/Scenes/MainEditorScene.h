#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class RenderSystem;
	class NativeScriptSystem;
	class MainEditor;

	class MainEditorScene : public Scene
	{
	public:
		MainEditorScene();
		virtual ~MainEditorScene();

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;
		virtual void OnEvent(Event& e) override;

		virtual void PostLoad() override;

	private:
		void OnUpdateEditor(DeltaTime dt);
		void OnUpdateRuntime(DeltaTime dt);
		void OnRenderEditor(const EditorCamera& camera);
		void OnRenderRuntime();

	private:
		Ref<RenderSystem> m_RenderSystem;
		Ref<NativeScriptSystem> m_NativeScriptSystem;

		MainEditor* m_MainEditor = nullptr;
	};

}
