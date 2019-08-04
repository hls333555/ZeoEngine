#include "HBestEngine.h"

class ExampleLayer : public HBestEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		HBE_INFO("ExampleLayer::Update");
	}

	void OnEvent(HBestEngine::Event& event) override
	{
		HBE_TRACE("{0}", event);
	}

};

class SandBox : public HBestEngine::Application
{
public:
	SandBox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new HBestEngine::ImGuiLayer());
	}

	~SandBox()
	{

	}

};

HBestEngine::Application* HBestEngine::CreateApplication()
{
	return new SandBox();
}
