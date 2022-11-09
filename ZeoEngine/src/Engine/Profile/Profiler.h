#pragma once

#include <optick.h>

#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Profile/RenderDoc.h"

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

namespace ZeoEngine {

	class Optick
	{
	public:
		void StartCapture(std::string name);
		void StopCapture() const;

	private:
		std::string m_CaptureName;
	};

	class Profiler
	{
	public:
		Profiler();

		PerformanceProfiler& GetPerformanceProfiler() { return m_PerfProfiler; }
		RenderDoc& GetRenderDoc() { return m_RenderDoc; }

	private:
		PerformanceProfiler m_PerfProfiler;
		Optick m_Optick;
		RenderDoc m_RenderDoc;
	};

}
