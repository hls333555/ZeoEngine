#include "ZEpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace ZeoEngine {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}
