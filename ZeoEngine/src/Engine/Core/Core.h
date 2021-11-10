#pragma once

#include <memory>
#include <string>

#include "Engine/Core/PlatformDetection.h"

#ifdef ZE_DEBUG
	#if defined(ZE_PLATFORM_WINDOWS)
		#define ZE_DEBUGBREAK() __debugbreak()
	#elif defined(ZE_PLATFORM_LINUX)
		#include <signal.h>
		#define ZE_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define ZE_ENABLE_ASSERTS
#else
	#define ZE_DEBUGBREAK()
#endif

#define ZE_WITH_EDITOR	1
#define ZE_SHOW_IMGUI_DEMO 0

namespace ZeoEngine {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// Makes unordered map keys case insensitive
	struct CaseInsensitiveUnorderedMap
	{
		struct Hash
		{
			std::size_t operator() (std::string str) const
			{
				for (std::size_t index = 0; index < str.size(); ++index)
				{
					auto ch = static_cast<unsigned char>(str[index]);
					str[index] = static_cast<unsigned char>(std::tolower(ch));
				}
				return std::hash<std::string>{}(str);
			}
		};
		struct Comp
		{
			bool operator() (const std::string& lhs, const std::string& rhs) const
			{
				// TODO: On non Windows OS, use the function "strcasecmp" in #include <strings.h>
				return _stricmp(lhs.c_str(), rhs.c_str()) == 0;
			}
		};
	};

}

#include "Engine/Core/Log.h"
#include "Engine/Core/Assert.h"
