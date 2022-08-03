#include "ZEpch.h"
#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Core/RandomEngine.h"

namespace ZeoEngine {

	EngineLayer::EngineLayer(const std::string& name)
		: Layer(name)
	{
		RandomEngine::Init();
	}

}
