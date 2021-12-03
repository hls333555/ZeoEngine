#pragma once

#include <magic_enum.hpp>

#include "Engine/Core/Application.h"

namespace ZeoEngine {

	class EngineUtils
	{
	public:
		// TODO:
		template<typename T>
		static const char* GetNameFromEnumType(T type)
		{
			return magic_enum::enum_name(type).data();
		}

		static float GetTimeInSeconds()
		{
			return Application::Get().GetTimeInSeconds();
		}

	};

}
