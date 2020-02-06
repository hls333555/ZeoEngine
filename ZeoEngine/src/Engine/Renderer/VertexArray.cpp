#include "ZEpch.h"
#include "Engine/Renderer/VertexArray.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace ZeoEngine {

	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexArray>();
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
