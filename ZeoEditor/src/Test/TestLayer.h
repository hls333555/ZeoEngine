#pragma once

#include "Engine/GameFramework/EngineLayer.h"

namespace ZeoEngine {

	class TestLayer : public EngineLayer
	{
	public:
		TestLayer();

		virtual void OnAttach() override;

	};

}
