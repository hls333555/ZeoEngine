#pragma once

#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "Engine/Debug/Instrumentor.h"

#ifdef ZE_PLATFORM_WINDOWS
//extern ZeoEngine::Application* ZeoEngine::CreateApplication();
int main(int argc, char** argv)
{
	doctest::Context context(argc, argv);
	int doctestRes = context.run();

	if (context.shouldExit())	// important - query flags (and --exit) rely on the user doing this
		return doctestRes;		// propagate the result of the tests

	ZeoEngine::Log::Init();
	ZE_CORE_TRACE("Initializing engine");
	ZE_CORE_TRACE("Initializing log");

	ZE_PROFILE_BEGIN_SESSION("Startup", "ZeoEngineProfile_Startup.json");
	auto app = ZeoEngine::CreateApplication();
	ZE_PROFILE_END_SESSION();

	ZE_PROFILE_BEGIN_SESSION("Runtime", "ZeoEngineProfile_Runtime.json");
	app->Run();
	ZE_PROFILE_END_SESSION();

	ZE_PROFILE_BEGIN_SESSION("Shutdown", "ZeoEngineProfile_Shutdown.json");
	delete app;
	ZE_PROFILE_END_SESSION();
}
#else
	#error ZeoEngine only supports Windows!
#endif // ZE_PLATFORM_WINDOWS
