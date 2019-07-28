#include "HBestEngine.h"

class SandBox : public HBestEngine::Application
{
public:
	SandBox()
	{

	}

	~SandBox()
	{

	}

};

HBestEngine::Application* HBestEngine::CreateApplication()
{
	return new SandBox();
}
