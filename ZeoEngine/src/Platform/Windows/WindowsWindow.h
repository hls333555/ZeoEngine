#pragma once

#include "Engine/Core/Window.h"

#include "Engine/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace ZeoEngine {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		virtual inline unsigned int GetWidth() const override { return m_Data.Width; }
		virtual inline unsigned int GetHeight() const override { return m_Data.Height; }

		virtual void OnUpdate() override;

		// Window attributes
		virtual inline void SetEventCallback(const EventCallbackFunc& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool bEnabled) override;
		virtual bool IsVSync() const override;

		virtual inline void* GetNativeWindow() const override { return m_Window; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool bVSync;

			EventCallbackFunc EventCallback;
		};

		WindowData m_Data;
	};

}
