#include "ZEpch.h"
#include "WindowsWindow.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace ZeoEngine {

	static bool s_bGLFWInitialized = false;

	static void GLFWErrorCallback(int error_code, const char* description)
	{
		ZE_CORE_ERROR("GLFW Error ({0}): {1}", error_code, description);
	}

	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<WindowsWindow>(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		ZE_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		ZE_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		ZE_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		ZE_CORE_INFO("Creating window: {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_bGLFWInitialized)
		{
			ZE_PROFILE_SCOPE("glfwInit");

			int success = glfwInit();
			ZE_CORE_ASSERT(success, "Failed to initialize GLFW!");
			// Set the GLFW error callback
			glfwSetErrorCallback(GLFWErrorCallback);

			s_bGLFWInitialized = true;
		}

		{
			ZE_PROFILE_SCOPE("glfwCreateWindow");

			m_Window = glfwCreateWindow(static_cast<int>(props.Width), static_cast<int>(props.Height), props.Title.c_str(), nullptr, nullptr);
			// Maximize window by default
			glfwMaximizeWindow(m_Window);
		}
		
		// Create rendering context
		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		// Pass in the window data which will be used in the following callbacks
		// This way, we do not need to capture the m_Data for lambda functions
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(false);

		// ---Set GLFW callbacks------------------------------------------------------------------------------------

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
				data.EventCallback(event);
				break;
			}
			default:
				break;
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int key) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			KeyTypedEvent event(key);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			default:
				break;
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			MouseScrolledEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
			data.EventCallback(event);
		});
	}

	void WindowsWindow::Shutdown()
	{
		ZE_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void WindowsWindow::OnUpdate()
	{
		ZE_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool bEnabled)
	{
		ZE_PROFILE_FUNCTION();

		if (bEnabled)
		{
			glfwSwapInterval(1);

		}
		else
		{
			glfwSwapInterval(0);
		}

		m_Data.bVSync = bEnabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.bVSync;
	}

}
