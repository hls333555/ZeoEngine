#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class MainEditorScene : public Scene
	{
	public:
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnSceneRender() override;

		virtual void OnDeserialized() override;

	private:
		virtual void OnClenup() override;
	};

}
