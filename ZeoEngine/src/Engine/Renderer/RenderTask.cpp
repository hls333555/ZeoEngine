#include "ZEpch.h"
#include "Engine/Renderer/RenderTask.h"

#include "Engine/Renderer/Drawable.h"
#include "Engine/Renderer/RenderStep.h"
#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	RenderTask::RenderTask(const Drawable* drawable, const RenderStep* step)
		: m_Drawable(drawable), m_Step(step)
	{
	}

	void RenderTask::Execute() const
	{
		m_Drawable->Bind();
		m_Step->Bind();
		RenderCommand::DrawIndexed(m_Drawable->GetBaseVertex(), m_Drawable->GetIndexCount(), m_Drawable->GetBaseIndex());
	}

}
