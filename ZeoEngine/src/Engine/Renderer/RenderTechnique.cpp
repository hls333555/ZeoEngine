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

	void RenderTechnique::UpdateContext(const Weak<Scene>& sceneContext, const Ref<Material>& material)
	{
		for (auto& step : m_Steps)
		{
			step.SetContext(sceneContext, material);
		}
	}

	void RenderTechnique::Submit(const Drawable& drawable)
	{
		if (m_bActive)
		{
			for (auto& step : m_Steps)
			{
				step.Submit(drawable);
			}
		}
	}

}
