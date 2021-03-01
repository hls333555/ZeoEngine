#pragma once

#include "Engine/Core/Layer.h"

namespace ZeoEngine {

	class EngineLayer : public Layer
	{
	public:
		EngineLayer(const std::string& name = "Layer");

		virtual void OnAttach() override;

	};

}
