#include "ZEpch.h"
#include "Engine/Renderer/RenderPassOutput.h"

namespace ZeoEngine {

	RenderPassOutput::RenderPassOutput(std::string name)
		: m_Name(std::move(name))
	{
		if (m_Name.empty())
		{
			ZE_CORE_ASSERT(false, "Render pass output name should not be empty!");
		}
	}

	Ref<BufferResource> RenderPassOutput::GetBufferResource() const
	{
		ZE_CORE_ASSERT(false, "Output cannot be accessed as a buffer resource!");
		return {};
	}

	Ref<Bindable> RenderPassOutput::GetBindable() const
	{
		ZE_CORE_ASSERT(false, "Output cannot be accessed as a bindable!");
		return {};
	}

}
