#pragma once

#include "Engine/Core/Layer.h"

namespace ZeoEngine {

	class GameLayer : public Layer
	{
	public:
		GameLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;

	};

}
