#pragma once

#include "Engine/Core/Core.h"

#include "Engine/Core/Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/EngineLayer.h"

namespace ZeoEngine {

	class ImGuiLayer;

	class Application
	{
	public:
		Application(const std::string& name = "Zeo App");
		virtual ~Application();

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_Window; }

		void Run();

		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		EngineLayer* GetEngineLayer();

		template<typename T>
		T* FindLayerByName(const std::string& layerName)
		{
			for (auto* layer : m_LayerStack)
			{
				if (layer->GetName() == layerName)
				{
					return dynamic_cast<T*>(layer);
				}
			}

			return nullptr;
		}

	private:
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
