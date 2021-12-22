#include "ZEpch.h"
#include "Engine/Renderer/BindableStates.h"

#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	void Depth::Bind() const
	{
		switch (m_State)
		{
			case Depth::State::ReadWrite:
				RenderCommand::ToggleDepthTesting(true);
				RenderCommand::ToggleDepthWriting(true);
				break;
			case Depth::State::ReadOnly:
				RenderCommand::ToggleDepthTesting(true);
				RenderCommand::ToggleDepthWriting(false);
				break;
			case Depth::State::Disable:
				RenderCommand::ToggleDepthTesting(false);
				break;
		}
	}

	void TwoSided::Bind() const
	{
		switch (m_State)
		{
		case TwoSided::State::CullFront:
			RenderCommand::ToggleFaceCulling(true);
			RenderCommand::SetFaceCullingMode(false);
			break;
		case TwoSided::State::CullBack:
			RenderCommand::ToggleFaceCulling(true);
			RenderCommand::SetFaceCullingMode(true);
			break;
		case TwoSided::State::Disable:
			RenderCommand::ToggleFaceCulling(false);
			break;
		}
	}

}
