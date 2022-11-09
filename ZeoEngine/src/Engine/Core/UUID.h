#pragma once

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class UUID
	{
	public:
		UUID();
		UUID(U64 uuid);
		UUID(const UUID&) = default;

		operator U64() const { return m_UUID; }

	private:
		U64 m_UUID = 0;
	};

}

namespace std {

	template<>
	struct hash<ZeoEngine::UUID>
	{
		ZeoEngine::SizeT operator()(const ZeoEngine::UUID& uuid) const noexcept
		{
			return uuid;
		}
	};

}
