#include "ZEpch.h"
#include "Engine/Core/Application.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/ImGui/ImGuiLayer.h"

#include <GLFW/glfw3.h>

extern "C"
{
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

namespace ZeoEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		ZE_PROFILE_FUNCTION();

		ZE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(ZE_BIND_EVENT_FUNC(Application::OnEvent));

		Renderer::Init();
		
		// m_ImGuiLayer does not need to be unique pointer
		// since it is going to be part of the layer stack who will control its lifecycle
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{
		ZE_PROFILE_FUNCTION();

		Renderer::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		ZE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowResize));

		// Iterate through the layer stack in a reverse order (from top to bottom) and break if current event is handled
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			// As of now, Application::OnWindowClose(WindowCloseEvent& e) will always return true,
			// so in this case, we want to first check if it is handled, and breaking before passing it through that layer
			if (e.m_bHandled) break;

			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		ZE_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		ZE_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	EngineLayer* Application::GetEngineLayer()
	{
		for (auto* layer : m_LayerStack)
		{
			return dynamic_cast<EngineLayer*>(layer);
		}
		return nullptr;
	}

	void Application::Run()
	{
		ZE_PROFILE_FUNCTION();

		while (m_bRunning)
		{
			ZE_PROFILE_SCOPE("RunLoop");

			// Platform::GetTime();
			float time = static_cast<float>(glfwGetTime());
			DeltaTime dt = time - m_LastFrameTime;
			m_LastFrameTime = time;

			// Stop updating layers if window is minimized
			if (!m_bMinimized)
			{
				{
					ZE_PROFILE_SCOPE("LayerStack OnUpdate");

					for (auto* layer : m_LayerStack)
					{
						layer->OnUpdate(dt);
					}
				}

				// TODO: This will eventually be in render thread
				// Render ImGui
				m_ImGuiLayer->Begin();
				{
					ZE_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (auto* layer : m_LayerStack)
					{
						layer->OnImGuiRender();
					}
				}
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
		}
	}

	void Application::Close()
	{
		m_bRunning = false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_bRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		ZE_PROFILE_FUNCTION();

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

