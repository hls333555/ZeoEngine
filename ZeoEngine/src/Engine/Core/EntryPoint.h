#pragma once

#include "Engine/Core/Application.h"

#ifdef ZE_PLATFORM_WINDOWS

extern ZeoEngine::Application* ZeoEngine::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	auto app = ZeoEngine::CreateApplication({ argc, argv });
	app->Run();
	delete app;
}
#else
	#error ZeoEngine only supports Windows!
#endif // ZE_PLATFORM_WINDOWS
