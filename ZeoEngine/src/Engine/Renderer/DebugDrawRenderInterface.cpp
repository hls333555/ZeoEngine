#include "ZEpch.h"
#include "Engine/Renderer/DebugDrawRenderInterface.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLDebugDrawRenderInterface.h"

namespace ZeoEngine {

	void DDRenderInterface::Init(SceneContext* sceneContext)
	{
		auto& context = sceneContext->DebugDrawContext;
		dd::initialize(&context, this);
		m_Context = context;
	}

	void DDRenderInterface::Flush(float timeInMs) const
	{
		dd::flush(m_Context, static_cast<I64>(timeInMs));
	}

	void DDRenderInterface::Shutdown() const
	{
		dd::shutdown(m_Context);
	}

	Ref<DDRenderInterface> DDRenderInterface::Create(SceneRenderer* sceneRenderer)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLDDRenderInterface>(sceneRenderer);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

}