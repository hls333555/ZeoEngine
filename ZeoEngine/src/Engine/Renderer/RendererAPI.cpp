#include "ZEpch.h"
#include "Engine/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace ZeoEngine {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateScope<OpenGLRendererAPI>();
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
