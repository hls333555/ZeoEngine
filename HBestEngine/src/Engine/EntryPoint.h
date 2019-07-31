#pragma once

// This entire file will be copied into HBestEngine.h due to #include "EntryPoint.h" there

#ifdef HBE_PLATFORM_WINDOWS
//extern HBestEngine::Application* HBestEngine::CreateApplication();
int main(int argc, char** argv)
{
	HBestEngine::Log::Init();
	HBE_CORE_TRACE("Initializing engine...");
	HBE_CORE_TRACE("Initialized log!");

	auto app = HBestEngine::CreateApplication();
	app->Run();
	delete app;
}
#else
	#error HBestEngine only supports Windows!
#endif // HBE_PLATFORM_WINDOWS
