#pragma once

#include <magic_enum.hpp>

namespace ZeoEngine {

	class EngineUtils
	{
	public:
		template<typename T>
		static const char* GetNameFromEnumType(T type)
		{
			return magic_enum::enum_name(type).data();
		}

	};

}
