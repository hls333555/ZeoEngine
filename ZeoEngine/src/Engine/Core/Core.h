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
	#define ZE_ASSERT(x, ...) { if(!(x)) { ZE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define ZE_CORE_ASSERT(x, ...) { if(!(x)) { ZE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define ZE_ASSERT(x, ...)
	#define ZE_CORE_ASSERT(x, ...)
#endif // ZE_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define ZE_BIND_EVENT_FUNC(func) std::bind(&func, this, std::placeholders::_1)

namespace ZeoEngine {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}
