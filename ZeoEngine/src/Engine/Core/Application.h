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

	class Application
	{
		friend int ::main(int argc, char** argv);

	public:
		Application(const std::string& name = "Zeo App");
		virtual ~Application();

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_Window; }

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

	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_bRunning = true;
		bool m_bMinimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;

	};

	// To be defined in the CLIENT
	Application* CreateApplication();
}
