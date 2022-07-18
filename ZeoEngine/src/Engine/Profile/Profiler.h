#pragma once

#include <optick.h>

#define ZE_ENABLE_PROFILING 1
#if ZE_ENABLE_PROFILING
	#define ZE_PROFILE_FRAME(...)           OPTICK_FRAME(__VA_ARGS__)
	#define ZE_PROFILE_FUNC(...)            OPTICK_EVENT(__VA_ARGS__)
	#define ZE_PROFILE_TAG(NAME, ...)       OPTICK_TAG(NAME, __VA_ARGS__)
	#define ZE_PROFILE_SCOPE_DYNAMIC(NAME)  OPTICK_EVENT_DYNAMIC(NAME)
	#define ZE_PROFILE_THREAD(...)          OPTICK_THREAD(__VA_ARGS__)
#else
	#define ZE_PROFILE_FRAME(...)
	#define ZE_PROFILE_FUNC()
	#define ZE_PROFILE_TAG(NAME, ...) 
	#define ZE_PROFILE_SCOPE_DYNAMIC(NAME)
	#define ZE_PROFILE_THREAD(...)
#endif
