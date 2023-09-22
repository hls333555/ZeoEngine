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
		Log::Init();

		ZE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Set working directory
		if (!m_Spec.WorkingDirectory.empty())
		{
			std::filesystem::current_path(m_Spec.WorkingDirectory);
		}

		m_Window = Window::Create(WindowProps(spec.Name));
		m_Window->SetEventCallback([this](Event& e) {return OnEvent(e); });
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

		// Clear scene references (so that physics scene can be destroyed) before physics engine shutdown
		// This fixes a crash when closing application during physics simulation
		ScriptEngine::Shutdown();
		// Clear assets especially PhysicsMaterial, so that releasing it won't crash
		AssetLibrary::Clear();
		PhysicsEngine::Shutdown();
		Renderer::Shutdown();
		TypeRegistry::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e){ return OnWindowClose(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
		dispatcher.Dispatch<WindowFocusChangedEvent>([this](WindowFocusChangedEvent& e) { return OnWindowFocusChanged(e); });

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
			const float time = m_Window->GetTimeInSeconds();
			DeltaTime dt = time - m_LastFrameTime;
			if (dt > 0.1f)
			{
				dt = 0.1f;
			}
			ZE_CORE_ASSERT(dt >= 0.0f);
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			// Stop updating layers if window is minimized
			if (!m_bMinimized)
			{
				{
					ZE_PROFILE_FUNC("Application::UpdateLayers");
					ZE_SCOPE_PERF("Application::UpdateLayers");

					for (auto* layer : m_LayerStack)
					{
						layer->OnUpdate(dt);
					}
				}

				// TODO: This will eventually be in render thread
				// Render ImGui
				{
					ZE_PROFILE_FUNC("Application::ImGuiRenderLayers");
					ZE_SCOPE_PERF("Application::ImGuiRenderLayers");

					m_ImGuiLayer->Begin();
					{
						for (auto* layer : m_LayerStack)
						{
							layer->OnImGuiRender();
						}

						if (m_ImGuiLayer->m_bShowImGuiStats)
						{
							ImGui::ShowMetricsWindow(&m_ImGuiLayer->m_bShowImGuiStats);
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

