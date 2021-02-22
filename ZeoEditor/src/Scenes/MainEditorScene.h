#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class MainEditorScene : public Scene
	{
	public:
		virtual ~MainEditorScene();

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;
		virtual void OnEvent(Event& e) override;

		virtual void OnDeserialized() override;

	private:
		virtual void OnClenup() override;

		void OnUpdateEditor(DeltaTime dt);
		void OnUpdateRuntime(DeltaTime dt);
		void OnRenderEditor(const EditorCamera& camera);
		void OnRenderRuntime();

		void UpdateParticleSystem(DeltaTime dt);
		void RenderPrimitives();
	};

}
