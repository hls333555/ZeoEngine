#pragma once

#include "Core.h"

#include "Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Layer.h"
#include "LayerStack.h"

namespace HBestEngine
{
	class HBE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_bRunning = true;
		LayerStack m_LayerStack;

	};

	// To be defined in the CLIENT
	Application* CreateApplication();
}
