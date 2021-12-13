#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticleEditorScene : public Scene
	{
	public:
		ParticleEditorScene();

		virtual void OnAttach() override;
		virtual void PostLoad() override;
	};

}
