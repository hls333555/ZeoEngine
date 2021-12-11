#include "ZEpch.h"
#include "Engine/Renderer/RenderTechnique.h"

namespace ZeoEngine {

	RenderTechnique::RenderTechnique(std::string name, bool bStartActive)
		: m_Name(std::move(name)), m_bActive(bStartActive)
	{
	}

	void RenderTechnique::AddStep(RenderStep step)
	{
		m_Steps.emplace_back(std::move(step));
	}

	void RenderTechnique::Link()
	{
		for (auto& step : m_Steps)
		{
			step.LinkRenderQueuePass();
		}
	}

	void RenderTechnique::Submit(const Drawable& drawable) const
	{
		if (m_bActive)
		{
			for (const auto& step : m_Steps)
			{
				step.Submit(drawable);
			}
		}
	}

}
