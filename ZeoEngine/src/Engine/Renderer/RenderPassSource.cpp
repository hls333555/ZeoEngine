#include "ZEpch.h"
#include "Engine/Renderer/RenderPassSource.h"

namespace ZeoEngine {

	RenderPassSource::RenderPassSource(std::string name)
		: m_Name(name)
	{
		if (m_Name.empty())
		{
			ZE_CORE_ASSERT(false, "Render pass source name should not be empty!");
		}
	}

	Ref<BufferResource> RenderPassSource::GetBufferResource() const
	{
		ZE_CORE_ASSERT(false, "Output cannot be accessed as a buffer resource!");
		return {};
	}

	Ref<Bindable> RenderPassSource::GetBindable() const
	{
		ZE_CORE_ASSERT(false, "Output cannot be accessed as a bindable!");
		return {};
	}

}
