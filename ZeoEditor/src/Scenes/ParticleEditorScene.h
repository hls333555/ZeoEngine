#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticleEditorScene : public Scene
	{
	public:
		virtual void OnAttach() override;
		virtual void PostLoad() override;
	};

}
