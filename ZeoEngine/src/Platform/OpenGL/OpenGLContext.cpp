#include "ZEpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace ZeoEngine {

	OpenGLContext::OpenGLContext(GLFWwindow* WindowHandle)
		: m_WindowHandle(WindowHandle)
	{
		ZE_CORE_ASSERT(WindowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		ZE_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGL();
		ZE_CORE_ASSERT(status, "Failed to initialize Glad!");

		ZE_CORE_INFO("Vendor: {0}", glGetString(GL_VENDOR));
		ZE_CORE_INFO("Renderer: {0}", glGetString(GL_RENDERER));
		ZE_CORE_INFO("Version: {0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		ZE_PROFILE_FUNCTION();
		
		glfwSwapBuffers(m_WindowHandle);
	}

}
