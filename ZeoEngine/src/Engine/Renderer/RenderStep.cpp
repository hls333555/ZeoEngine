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

	void RenderStep::LinkRenderQueuePass()
	{
		ZE_CORE_ASSERT(!m_RenderQueuePass);

		//m_RenderQueuePass = Renderer::GetActiveRenderGraph()->GetRenderQueuePass(m_RenderQueuePassName);
	}

	void RenderStep::Bind() const
	{
		for (const auto& bindable : m_Bindables)
		{
			bindable->Bind();
		}
	}

	void RenderStep::Submit(const Drawable& drawable) const
	{
		// TODO:
		Renderer::GetActiveRenderGraph()->GetRenderQueuePass(m_RenderQueuePassName)->AddTask(RenderTask(&drawable, this));
	}

}
