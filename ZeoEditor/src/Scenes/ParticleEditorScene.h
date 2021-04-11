#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticleEditorScene : public Scene
	{
	public:
		ParticleEditorScene() = default;

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;
	};

}
