#include "ZEpch.h"
#include "Engine/Renderer/RenderStep.h"

#include "Engine/Renderer/Bindable.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Renderer/RenderGraph.h"
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

	void RenderStep::SetContext(const Weak<Scene>& sceneContext, const AssetHandle<Material>& material)
	{
		m_SceneContext = sceneContext;
		m_MaterialRef = material;
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

	void RenderStep::LinkRenderQueuePass()
	{
		const auto& sceneRenderer = EngineUtils::GetSceneRendererFromContext(m_SceneContext.lock());
		ZE_CORE_ASSERT(sceneRenderer);

		m_RenderQueuePass = sceneRenderer->GetRenderGraph().GetRenderQueuePass(m_RenderQueuePassName);
	}

	void RenderStep::Submit(const Drawable& drawable)
	{
		if (!m_RenderQueuePass)
		{
			LinkRenderQueuePass();
			ZE_CORE_ASSERT(m_RenderQueuePass);
		}
		m_RenderQueuePass->AddTask(&drawable, this);
	}

}
