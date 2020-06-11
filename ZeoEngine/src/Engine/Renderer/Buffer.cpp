#include "ZEpch.h"
#include "Engine/Renderer/Buffer.h"

#include "Engine/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace ZeoEngine {

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(vertices, size);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpec& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLFrameBuffer>(spec);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
