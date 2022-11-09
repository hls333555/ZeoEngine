#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"

struct GLFWwindow;

namespace ZeoEngine {

	struct WindowProps
	{
		std::string Title;
		U32 Width;
		U32 Height;

		WindowProps(const std::string& title = "Zeo Engine",
			U32 width = 1920,
			U32 height = 1080)
			: Title(title), Width(width), Height(height) {}
	};

	/** Interface representing a desktop system based Window */
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;

		virtual float GetTimeInSeconds() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool bEnabled) = 0;
		virtual bool IsVSync() const = 0;

		/** Returns the actual window based on the platform. */
		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());

		static void SetIcon(GLFWwindow* window);
		static void LockMouse(GLFWwindow* window, bool bLock);
	
	};

}
