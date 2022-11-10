#include "ZEpch.h"
#include "Engine/Core/Application.h"

#include <GLFW/glfw3.h>

#include "Engine/Core/RandomEngine.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/GameFramework/TypeRegistry.h"
#include "Engine/ImGui/ImGuiLayer.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Profile/Profiler.h"

extern "C"
{
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

namespace ZeoEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
		: m_Spec(spec)
	{
		ZE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Set working directory
		if (!m_Spec.WorkingDirectory.empty())
		{
			std::filesystem::current_path(m_Spec.WorkingDirectory);
		}

		m_Profiler = new Profiler();

		m_Window = Window::Create(WindowProps(spec.Name));
		m_Window->SetEventCallback(ZE_BIND_EVENT_FUNC(Application::OnEvent));
		m_ActiveWindow = static_cast<GLFWwindow*>(m_Window->GetNativeWindow());

		// TODO:
		RandomEngine::Init();
		Renderer::Init();
		ScriptEngine::Init();
		PhysicsEngine::Init();
		TypeRegistry::Init();
		
		// m_ImGuiLayer does not need to be unique pointer
		// since it is going to be part of the layer stack who will control its lifecycle
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{
		for (auto* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		ScriptEngine::Shutdown();
		// Clear scene references (so that physics scene can be destroyed) before physics engine shutdown
		// This fixes a crash when closing application during physics simulation
		PhysicsEngine::Shutdown();
		Renderer::Shutdown();

		delete m_Profiler;
		m_Profiler = nullptr;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowResize));
		dispatcher.Dispatch<WindowFocusChangedEvent>(ZE_BIND_EVENT_FUNC(Application::OnWindowFocusChanged));

		PropagateEvent(e);
	}

	void Application::PropagateEvent(Event& e)
	{
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
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(func);
	}

	void Application::Run()
	{
		while (m_bRunning)
		{
			ZE_PROFILE_FRAME("MainThread");

			// Platform::GetTime();
			float time = m_Window->GetTimeInSeconds();
			DeltaTime dt = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			// Stop updating layers if window is minimized
			if (!m_bMinimized)
			{
				{
					ZE_PROFILE_FUNC("Application Layer::OnUpdate");
					ZE_SCOPE_PERF("Application Layer::OnUpdate");

					for (auto* layer : m_LayerStack)
					{
						layer->OnUpdate(dt);
					}
				}

				// TODO: This will eventually be in render thread
				// Render ImGui
				{
					ZE_PROFILE_FUNC("Application Layer::OnImGuiRender");
					ZE_SCOPE_PERF("Application Layer::OnImGuiRender");

					m_ImGuiLayer->Begin();
					{
						for (auto* layer : m_LayerStack)
						{
							layer->OnImGuiRender();
						}
					}
					m_ImGuiLayer->End();
				}
			}

			// Do swap buffers and other stuff
			m_Window->OnUpdate();
		}
	}

	void Application::Close()
	{
		m_bRunning = false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		if (e.GetWindow() != m_Window->GetNativeWindow()) return false;

		m_bRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWindow() != m_Window->GetNativeWindow()) return false;

		// When window is minimized...
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_bMinimized = true;
			// Hide other owned windows
			for (auto* window : m_ViewportWindows)
			{
				glfwHideWindow(window);
			}
			return false;
		}

		m_bMinimized = false;
		// Show other owned windows
		for (auto* window : m_ViewportWindows)
		{
			glfwShowWindow(window);
		}
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Application::OnWindowFocusChanged(WindowFocusChangedEvent& e)
	{
		if (e.IsFocused())
		{
			m_ActiveWindow = e.GetWindow();
		}
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue)
		{
			func();
		}
		m_MainThreadQueue.clear();
	}

}

