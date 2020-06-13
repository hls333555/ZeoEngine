#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "GameLayer.h"

class SandBox : public ZeoEngine::Application
{
public:
	SandBox()
	{
		PushLayer(new ZeoEngine::GameLayer());
	}

	~SandBox()
	{
	}

};

ZeoEngine::Application* ZeoEngine::CreateApplication()
{
	return new SandBox();
}
