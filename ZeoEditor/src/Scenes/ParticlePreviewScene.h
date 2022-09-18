#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticlePreviewScene : public Scene
	{
	public:
		virtual void OnAttach(const Ref<WorldBase>& world) override;
		virtual void PostLoad() override;
	};

}
