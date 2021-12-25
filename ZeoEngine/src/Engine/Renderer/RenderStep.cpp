#include "ZEpch.h"
#include "Engine/Renderer/RenderStep.h"

#include "Engine/Renderer/Bindable.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Renderer.h"

namespace ZeoEngine {

	RenderStep::RenderStep(std::string renderQueuePassName)
		: m_RenderQueuePassName(std::move(renderQueuePassName))
	{
	}

	void RenderStep::AddBindable(const Ref<Bindable>& bindable)
	{
		m_Bindables.emplace_back(bindable);
	}

	void RenderStep::LinkRenderQueuePass(const RenderGraph& renderGraph)
	{
		ZE_CORE_ASSERT(!m_RenderQueuePass);

		m_RenderQueuePass = renderGraph.GetRenderQueuePass(m_RenderQueuePassName);
	}

	void RenderStep::Bind() const
	{
		for (const auto& bindable : m_Bindables)
		{
			bindable->Bind();
		}
	}

	void RenderStep::Unbind() const
	{
		for (const auto& bindable : m_Bindables)
		{
			bindable->Unbind();
		}
	}

	void RenderStep::Submit(const Drawable& drawable) const
	{
		m_RenderQueuePass->AddTask(RenderTask(&drawable, this));
	}

}
