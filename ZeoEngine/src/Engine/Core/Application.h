#pragma once

#include "Engine/Core/Core.h"

#include "Engine/Core/Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/EngineLayer.h"

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

	class Application
	{
		friend int ::main(int argc, char** argv);

	public:
		Application(const std::string& name = "Zeo App", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
		virtual ~Application();

		static Application& Get() { return *s_Instance; }
		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }
		Window& GetWindow() { return *m_Window; }

		float GetTimeInSeconds() const { return m_Window->GetTimeInSeconds(); }

		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		template<typename T>
		T* FindLayer()
		{
			for (auto* layer : m_LayerStack)
			{
				if (T* layerCast = dynamic_cast<T*>(layer))
				{
					return layerCast;
				}
			}
			return nullptr;
		}

	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowFocusChanged(WindowFocusChangedEvent& e);

	private:
		ApplicationCommandLineArgs m_CommandLineArgs;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_bRunning = true;
		bool m_bMinimized = false;
		bool m_bFocused = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;

	};

	// To be defined in the CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
