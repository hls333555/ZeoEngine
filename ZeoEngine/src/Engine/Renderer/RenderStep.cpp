#include "ZEpch.h"
#include "Engine/Renderer/RenderStep.h"

#include "Engine/Renderer/Drawable.h"
#include "Engine/Renderer/Bindable.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	RenderStep::RenderStep(std::string renderQueuePassName)
		: m_RenderQueuePassName(std::move(renderQueuePassName))
	{
	}

	void RenderStep::AddBindable(const Ref<Bindable>& bindable)
	{
		m_Bindables.emplace_back(bindable);
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

	RenderStepInstance::RenderStepInstance(const RenderStep* step, const SceneContext* sceneContext)
		: m_Step(step)
	{
		m_RenderQueuePass = EngineUtils::GetRenderPassFromContext<RenderQueuePass>(sceneContext, m_Step->m_RenderQueuePassName);
	}

	void RenderStepInstance::Submit(const Drawable& drawable) const
	{
		m_RenderQueuePass->AddTask(&drawable, m_Step);
	}

}
