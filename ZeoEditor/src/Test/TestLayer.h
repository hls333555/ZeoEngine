#pragma once

#include "Engine/Core/Layer.h"

namespace ZeoEngine {

	class TestLayer : public Layer
	{
	public:
		TestLayer();

		virtual void OnAttach() override;

	};

}
