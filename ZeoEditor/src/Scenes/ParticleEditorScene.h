#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticlePreviewRenderSystem;

	class ParticleEditorScene : public Scene
	{
	public:
		ParticleEditorScene();
		virtual ~ParticleEditorScene();

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;

		virtual void PostLoad() override;

	private:
		Scope<ParticlePreviewRenderSystem> m_ParticlePreviewRenderSystem;
	};

}
