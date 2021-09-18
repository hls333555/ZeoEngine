#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticlePreviewRenderSystem;

	class ParticleScene : public Scene
	{
	public:
		ParticleScene();
		virtual ~ParticleScene();

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;

		virtual void PostLoad() override;

	private:
		Ref<ParticlePreviewRenderSystem> m_ParticlePreviewRenderSystem;
	};

}
