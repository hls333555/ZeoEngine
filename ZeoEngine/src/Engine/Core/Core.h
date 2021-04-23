#pragma once

#include <memory>

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
}

#include "Engine/Core/Log.h"
#include "Engine/Core/Assert.h"
