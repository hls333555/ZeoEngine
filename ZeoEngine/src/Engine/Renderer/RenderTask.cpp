#include "ZEpch.h"
#include "Engine/Renderer/RenderTask.h"

#include "Engine/Renderer/Drawable.h"
#include "Engine/Renderer/RenderStep.h"
#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	SubRenderTask::SubRenderTask(const Drawable* drawable)
		: m_Drawable(drawable)
	{
	}

	void SubRenderTask::Execute() const
	{
		m_Drawable->Bind();
		RenderCommand::DrawIndexed(m_Drawable->GetBaseVertex(), m_Drawable->GetIndexCount(), m_Drawable->GetBaseIndex());
	}

	RenderTask::RenderTask(const RenderStep* step)
		: m_Step(step)
	{
	}

	bool RenderTask::HasSameMaterial(const RenderStep* step) const
	{
		return m_Step->GetMaterialRef() == step->GetMaterialRef();
	}

	void RenderTask::AddSubTask(SubRenderTask subTask)
	{
		m_SubTasks.emplace_back(subTask);
	}

	void RenderTask::Execute() const
	{
		// Bind material
		m_Step->Bind();
		for (const auto& subTask : m_SubTasks)
		{
			subTask.Execute();
		}
		// TODO: Can remove?
		m_Step->Unbind();
	}

}
