#pragma once

#include <memory>

#ifdef ZE_PLATFORM_WINDOWS
	// Static linking is used from now on!
	#if ZE_DYNAMIC_LINK
		#ifdef ZE_BUILD_DLL
			#define ZE_API __declspec(dllexport)
		#else
			#define ZE_API __declspec(dllimport)
		#endif // ZE_BUILD_DLL
	#else
		#define ZE_API
	#endif // ZE_DYNAMIC_LINK
#else
	#error ZeoEngine only supports Windows!
#endif // ZE_PLATFORM_WINDOWS

#ifdef ZE_DEBUG
	#define ZE_ENABLE_ASSERTS
#endif // ZE_DEBUG

#ifdef ZE_ENABLE_ASSERTS
	#define ZE_ASSERT(x) { if(!(x)) { ZE_ERROR("Assertion Failed!"); __debugbreak(); } }
	#define ZE_ASSERT_INFO(x, ...) { if(!(x)) { ZE_CORE_ERROR("Assertion Failed:"); ZE_ERROR(__VA_ARGS__); __debugbreak(); } }
	#define ZE_CORE_ASSERT(x) { if(!(x)) { ZE_CORE_ERROR("Assertion Failed!"); __debugbreak(); } }
	#define ZE_CORE_ASSERT_INFO(x, ...) { if(!(x)) { ZE_CORE_ERROR("Assertion Failed:"); ZE_CORE_ERROR(__VA_ARGS__); __debugbreak(); } }
#else
	#define ZE_ASSERT(x)
	#define ZE_ASSERT_INFO(x, ...)
	#define ZE_CORE_ASSERT(x)
	#define ZE_CORE_ASSERT_INFO(x, ...)
#endif // ZE_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define ZE_BIND_EVENT_FUNC(func) std::bind(&func, this, std::placeholders::_1)

#ifndef WITH_EDITOR
// Set to 0 when toggling standalone game mode
	#define WITH_EDITOR	1
#endif // WITH_EDITOR

//#define ENUMCLASS_TO_INT(enumVar) static_cast<int32_t>(enumVar)

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
