#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class ParticleEditorScene : public Scene
	{
	public:
		virtual ~ParticleEditorScene();

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnRender(const EditorCamera& camera) override;

	private:
		virtual void OnClenup() override;
	};

}
