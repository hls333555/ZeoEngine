#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "GameLayer.h"

class SandBox : public ZeoEngine::Application
{
public:
	SandBox(ZeoEngine::ApplicationCommandLineArgs args)
		: Application("Sandbox", args)
	{
		PushLayer(new ZeoEngine::GameLayer());
	}

	~SandBox()
	{
	}

};

ZeoEngine::Application* ZeoEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	return new SandBox(args);
}
