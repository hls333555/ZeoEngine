#pragma once

#ifdef HBE_PLATFORM_WINDOWS
extern HBestEngine::Application* HBestEngine::CreateApplication();
int main(int argc, char** argv)
{
	auto app = HBestEngine::CreateApplication();
	app->Run();
	delete app;
}
#else
	#error HBestEngine only support Windows!
#endif // HBE_PLATFORM_WINDOWS
