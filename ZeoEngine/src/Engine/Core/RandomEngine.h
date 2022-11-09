#pragma once

#include <random>

#include <glm/glm.hpp>

namespace ZeoEngine {

	class RandomEngine
	{
	public:
		static void Init()
		{
			s_RandomEngine.seed(std::random_device()());
		}

		/** Returns a random float number in [0.0, 1.0]. */
		static float RandFloat()
		{
			return s_Distribution(s_RandomEngine);
		}
		/** Returns a random float number in [lower, upper]. */
		static float RandFloatInRange(float lower, float upper)
		{
			return s_Distribution(s_RandomEngine) * (upper - lower) + lower;
		}
		/** Returns a random int number in [0, upper). */
		static I32 RandInt(I32 upper)
		{
			return upper > 0 ? glm::min(static_cast<I32>(glm::trunc(RandFloat() * upper)), upper - 1) : 0;
		}
		/** Returns a random int number in [lower, upper]. */
		static I32 RandIntInRange(I32 lower, I32 upper)
		{
			const I32 range = (upper - lower) + 1;
			return lower + RandInt(range);
		}

	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_real_distribution<float> s_Distribution;
	};

}
