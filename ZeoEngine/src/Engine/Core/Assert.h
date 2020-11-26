#pragma once

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
