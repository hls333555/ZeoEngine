#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "GameLayer.h"

class SandBox : public ZeoEngine::Application
{
public:
	SandBox(const ZeoEngine::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new ZeoEngine::GameLayer());
	}

	~SandBox()
	{
	}

};

ZeoEngine::Application* ZeoEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../ZeoEditor";
	spec.CommandLineArgs = args;
	return new SandBox(spec);
}
