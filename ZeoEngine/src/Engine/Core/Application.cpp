#include "ZEpch.h"
#include "Application.h"

#include "Log.h"

#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace ZeoEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		ZE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = Scope<Window>(Window::Create());
		m_Window->SetEventCallback(ZE_BIND_EVENT_FUNC(Application::OnEvent));

		Renderer::Init();
		
		// m_ImGuiLayer does not need to be unique pointer
		// since it is going to be part of the layer stack who will control its lifecycle
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowResize));

		// Iterate through the layer stack in a reverse order (from top to bottom) and break if current event is handled
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.m_bHandled)
				break;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	void Application::Run()
	{
		while (m_bRunning)
		{
			// Platform::GetTime();
			float time = (float)glfwGetTime();
			DeltaTime dt = time - m_LastFrameTime;
			m_LastFrameTime = time;

			// Stop updating layers if window is minimized
			if (!m_bMinimized)
			{
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(dt);
				}
			}

			// TODO: This will eventually be in render thread
			// Render ImGui
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_bRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		// When window is minimized...
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_bMinimized = true;
			return false;
		}

		m_bMinimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}

