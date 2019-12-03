#pragma once

#ifdef ZE_PLATFORM_WINDOWS
//extern ZeoEngine::Application* ZeoEngine::CreateApplication();
int main(int argc, char** argv)
{
	ZeoEngine::Log::Init();
	ZE_CORE_TRACE("Initializing engine...");
	ZE_CORE_TRACE("Initialized log!");

	auto app = ZeoEngine::CreateApplication();
	app->Run();
	delete app;
}
#else
	#error ZeoEngine only supports Windows!
#endif // ZE_PLATFORM_WINDOWS
