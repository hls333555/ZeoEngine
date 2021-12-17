#include "ZEpch.h"
#include "Engine/Renderer/BindableStates.h"

#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	void Depth::Bind(uint32_t slot) const
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

	void TwoSided::Bind(uint32_t slot) const
	{
		RenderCommand::ToggleFaceCulling(!m_bEnable);
	}

}
