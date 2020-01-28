#pragma once

#include <random>

namespace ZeoEngine {

	class RandomEngine
	{
	public:
		static void Init()
		{
			s_RandomEngine.seed(std::random_device()());
		}

		static float RandFloat()
		{
			return s_Distribution(s_RandomEngine);
		}
		static float RandFloatInRange(float lower, float upper)
		{
			return s_Distribution(s_RandomEngine) * (upper - lower) + lower;
		}

	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_real_distribution<float> s_Distribution;
	};

}
