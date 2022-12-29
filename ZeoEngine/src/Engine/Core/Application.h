#pragma once

#include "Engine/Core/Core.h"

#include "Engine/Core/Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Profile/Profiler.h"

int main(int argc, char** argv);

namespace ZeoEngine {

	class ImGuiLayer;

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			ZE_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "ZeoEngine";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
		friend int ::main(int argc, char** argv);

	public:
		Application(const ApplicationSpecification& spec);
		virtual ~Application();

		static Application& Get() { return *s_Instance; }
		const auto& GetSpecification() const { return m_Spec; }
		Window& GetWindow() { return *m_Window; }
		GLFWwindow* GetActiveNativeWindow() const { return m_ActiveWindow; }

#ifndef ZE_DIST
		RenderDoc* GetRenderDoc() { return m_Profiler.GetRenderDoc(); }
		PerformanceProfiler* GetPerformanceProfiler() { return m_Profiler.GetPerformanceProfiler(); }
#else
		RenderDoc* GetRenderDoc() { return nullptr; }
		PerformanceProfiler* GetPerformanceProfiler() { return nullptr; }
#endif

		void AddViewportWindow(GLFWwindow* window) { m_ViewportWindows.emplace_back(window); }
		void RemoveViewportWindow(GLFWwindow* window) { m_ViewportWindows.erase(std::find(m_ViewportWindows.begin(), m_ViewportWindows.end(), window)); }

		float GetTimeInSeconds() const { return m_Window->GetTimeInSeconds(); }

		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }

		void SubmitToMainThread(const std::function<void()>& func);

	private:
		void Run();
		void PropagateEvent(Event& e);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowFocusChanged(WindowFocusChangedEvent& e);

		void ExecuteMainThreadQueue();

	private:
		ApplicationSpecification m_Spec;

		Scope<Window> m_Window;
		std::vector<GLFWwindow*> m_ViewportWindows; // TODO: Abstract GLFWwindow
		GLFWwindow* m_ActiveWindow = nullptr;

		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

		bool m_bRunning = true;
		bool m_bMinimized = false;
		float m_LastFrameTime = 0.0f;

#ifndef ZE_DIST
		Profiler m_Profiler;
#endif

		static Application* s_Instance;

	};

	// To be defined in the CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
