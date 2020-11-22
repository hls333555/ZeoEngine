#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticleEditorScene : public Scene
	{
	public:
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnSceneRender() override;

	private:
		virtual void OnClenup() override;
	};

}
