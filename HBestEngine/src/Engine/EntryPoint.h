#pragma once

// This entire file will be copied into HBestEngine.h due to #include "EntryPoint.h" there

#ifdef HBE_PLATFORM_WINDOWS
//extern HBestEngine::Application* HBestEngine::CreateApplication();
int main(int argc, char** argv)
{
	HBestEngine::Log::Init();
	HBE_CORE_TRACE("Initialized log!");
	HBE_CORE_INFO("Initialized log!");
	HBE_CORE_WARN("Initialized log!");
	HBE_CORE_ERROR("Initialized log!");
	HBE_CORE_FATAL("Initialized log!");
	int a = 5;
	HBE_TRACE("Hello! Var={0}", a);
	HBE_INFO("Hello! Var={0}", a);
	HBE_WARN("Hello! Var={0}", a);
	HBE_ERROR("Hello! Var={0}", a);
	HBE_FATAL("Hello! Var={0}", a);

	auto app = HBestEngine::CreateApplication();
	app->Run();
	delete app;
}
#else
	#error HBestEngine only support Windows!
#endif // HBE_PLATFORM_WINDOWS
