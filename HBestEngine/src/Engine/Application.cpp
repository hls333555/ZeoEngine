#include "HBEpch.h"
#include "Application.h"

#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace HBestEngine
{
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_bRunning)
		{
			m_Window->OnUpdate();
		}
	}
}

