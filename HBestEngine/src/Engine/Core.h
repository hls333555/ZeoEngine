#pragma once

#ifdef HBE_PLATFORM_WINDOWS
	#ifdef HBE_BUILD_DLL
		#define HBE_API __declspec(dllexport)
	#else
		#define HBE_API __declspec(dllimport)
	#endif // HBE_BUILD_DLL
#else
	#error HBestEngine only supports Windows!
#endif // HBE_PLATFORM_WINDOWS

#ifdef HZ_ENABLE_ASSERTS
	#define HBE_ASSERT(x, ...) { if(!(x)) { HBE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HBE_CORE_ASSERT(x, ...) { if(!(x)) { HBE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HBE_ASSERT(x, ...)
	#define HBE_CORE_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

#define BIT(x) (1 << x)
