#pragma once

#include "Engine/Core/Window.h"

#include "Engine/Renderer/GraphicsContext.h"

namespace ZeoEngine {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		virtual U32 GetWidth() const override { return m_Data.Width; }
		virtual U32 GetHeight() const override { return m_Data.Height; }

		virtual float GetTimeInSeconds() const override;

		virtual void OnUpdate() override;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool bEnabled) override;
		virtual bool IsVSync() const override;

		virtual void* GetNativeWindow() const override { return m_Window; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window = nullptr;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			U32 Width, Height;
			bool bVSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}
