#pragma once

#include <filesystem>

#include "Engine/Core/CoreMacros.h"
#include "Engine/Core/Log.h"

#ifdef ZE_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define _ZE_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { ZE##type##ERROR(msg, __VA_ARGS__); ZE_DEBUGBREAK(); } }
	#define _ZE_ASSERT_WITH_MSG(type, check, ...) _ZE_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define _ZE_ASSERT_WITH_MSG_ARGS(type, check, msg, ...) _ZE_ASSERT_IMPL(type, check, "Assertion failed: "##msg, __VA_ARGS__)
	#define _ZE_ASSERT_NO_MSG(type, check) _ZE_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}: {2}", ZE_STRINGIFY(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define _ZE_ASSERT_GET_MACRO_NAME(arg1, arg2, arg3, arg4, arg5, macro, ...) macro
	#define _ZE_ASSERT_GET_MACRO(...) ZE_EXPAND( _ZE_ASSERT_GET_MACRO_NAME(__VA_ARGS__, _ZE_ASSERT_WITH_MSG_ARGS, _ZE_ASSERT_WITH_MSG_ARGS, _ZE_ASSERT_WITH_MSG_ARGS, _ZE_ASSERT_WITH_MSG, _ZE_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and an optional message as well as up to three additional parameters being optional
	#define ZE_ASSERT(...) ZE_EXPAND( _ZE_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define ZE_CORE_ASSERT(...) ZE_EXPAND( _ZE_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define ZE_ASSERT(...)
	#define ZE_CORE_ASSERT(...)
#endif
