#include "ZEpch.h"
#include "Engine/Core/RandomEngine.h"

namespace ZeoEngine {

	std::mt19937 RandomEngine::s_RandomEngine;
	std::uniform_real_distribution<float> RandomEngine::s_Distribution(0, 1);

}
