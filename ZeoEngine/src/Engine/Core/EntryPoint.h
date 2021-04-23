#pragma once

#include "Engine/Debug/Instrumentor.h"

#ifdef ZE_PLATFORM_WINDOWS
//extern ZeoEngine::Application* ZeoEngine::CreateApplication();
int main(int argc, char** argv)
{
	ZeoEngine::Log::Init();

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
