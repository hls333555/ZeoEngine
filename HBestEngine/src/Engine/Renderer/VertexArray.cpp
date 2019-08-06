#include "HBEpch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace HBestEngine
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			HBE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::OpenGL:
			return new OpenGLVertexArray();
		default:
			HBE_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
