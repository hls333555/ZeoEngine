#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticlePreviewScene : public Scene
	{
	public:
		~ParticlePreviewScene() override;

		virtual void OnAttach(const Ref<WorldBase>& world) override;
		virtual void PostLoad() override;
	};

}
