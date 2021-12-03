#include "ZEpch.h"
#include "Engine/Renderer/DebugDrawRenderInterface.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLDebugDrawRenderInterface.h"

namespace ZeoEngine {

	void DDRenderInterface::Init(const Scope<DDRenderInterface>& ddri)
	{
		ddri->Init();
		dd::initialize(static_cast<dd::RenderInterface*>(ddri.get()));
	}

	void DDRenderInterface::Flush(float timeInMs)
	{
		dd::flush(static_cast<int64_t>(timeInMs));
	}

	void DDRenderInterface::Shutdown()
	{
		dd::shutdown();
	}

	Scope<DDRenderInterface> DDRenderInterface::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLDDRenderInterface>();
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

}