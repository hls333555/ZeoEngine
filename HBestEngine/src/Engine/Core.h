#pragma once

#include <memory>

#ifdef HBE_PLATFORM_WINDOWS
	// Static linking is used from now on!
	#if HBE_DYNAMIC_LINK
		#ifdef HBE_BUILD_DLL
			#define HBE_API __declspec(dllexport)
		#else
			#define HBE_API __declspec(dllimport)
		#endif // HBE_BUILD_DLL
	#else
		#define HBE_API
	#endif // HBE_DYNAMIC_LINK
#else
	#error HBestEngine only supports Windows!
#endif // HBE_PLATFORM_WINDOWS

#ifdef HBE_DEBUG
	#define HBE_ENABLE_ASSERTS
#endif // HBE_DEBUG

#ifdef HBE_ENABLE_ASSERTS
	#define HBE_ASSERT(x, ...) { if(!(x)) { HBE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HBE_CORE_ASSERT(x, ...) { if(!(x)) { HBE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HBE_ASSERT(x, ...)
	#define HBE_CORE_ASSERT(x, ...)
#endif // HBE_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define HBE_BIND_EVENT_FUNC(func) std::bind(&func, this, std::placeholders::_1)

namespace HBestEngine {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}
