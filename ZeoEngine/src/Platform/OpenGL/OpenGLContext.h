#pragma once

#include "Engine/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace ZeoEngine {

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* WindowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;

	};

}
