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

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		bool OnWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		bool m_bRunning = true;
		LayerStack m_LayerStack;
		static Application* s_Instance;

	};

	// To be defined in the CLIENT
	Application* CreateApplication();
}
