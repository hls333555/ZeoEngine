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
