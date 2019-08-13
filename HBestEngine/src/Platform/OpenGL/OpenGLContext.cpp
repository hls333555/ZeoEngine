#include "HBEpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace HBestEngine {

	OpenGLContext::OpenGLContext(GLFWwindow* WindowHandle)
		: m_WindowHandle(WindowHandle)
	{
		HBE_CORE_ASSERT(WindowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGL();
		HBE_CORE_ASSERT(status, "Failed to initialize Glad!");

		HBE_CORE_INFO("Vendor: {0}", glGetString(GL_VENDOR));
		HBE_CORE_INFO("Renderer: {0}", glGetString(GL_RENDERER));
		HBE_CORE_INFO("Version: {0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}
