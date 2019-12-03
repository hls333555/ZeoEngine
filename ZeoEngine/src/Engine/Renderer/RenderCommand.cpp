#include "ZEpch.h"
#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}
