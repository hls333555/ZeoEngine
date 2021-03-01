#pragma once

#include "Engine/GameFramework/EngineLayer.h"

namespace ZeoEngine {

	class GameLayer : public EngineLayer
	{
	public:
		GameLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;

	};

}
