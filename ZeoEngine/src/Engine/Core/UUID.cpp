#include "ZEpch.h"
#include "Engine/Core/UUID.h"

#include <random>

namespace ZeoEngine {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<U64> s_UniformDistribution;

	UUID::UUID()
		: m_UUID(s_UniformDistribution(s_Engine))
	{
	}

	UUID::UUID(U64 uuid)
		: m_UUID(uuid)
	{
	}

}
