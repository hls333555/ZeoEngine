#pragma once

#include "Engine/Window.h"

#include <GLFW/glfw3.h>

namespace HBestEngine
{
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

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;

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
