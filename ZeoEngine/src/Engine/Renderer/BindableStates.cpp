#include "ZEpch.h"
#include "Engine/Renderer/BindableStates.h"

#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	void Depth::Bind() const
	{
		switch (m_State)
		{
			case State::ReadWrite:
				RenderCommand::ToggleDepthTesting(true);
				RenderCommand::ToggleDepthWriting(true);
				break;
			case State::ReadOnly:
				RenderCommand::ToggleDepthTesting(true);
				RenderCommand::ToggleDepthWriting(false);
				break;
			case State::Disable:
				RenderCommand::ToggleDepthTesting(false);
				break;
			case State::ToggleClamp:
				RenderCommand::ToggleDepthClamping(true);
				break;
		}
	}

	void Depth::Unbind() const
	{
		switch (m_State)
		{
			case State::ToggleClamp:
				RenderCommand::ToggleDepthClamping(false);
				break;
		}
	}

	void TwoSided::Bind() const
	{
		switch (m_State)
		{
			case State::CullFront:
				RenderCommand::ToggleFaceCulling(true);
				RenderCommand::SetFaceCullingMode(false);
				break;
			case State::CullBack:
				RenderCommand::ToggleFaceCulling(true);
				RenderCommand::SetFaceCullingMode(true);
				break;
			case State::Disable:
				RenderCommand::ToggleFaceCulling(false);
				break;
		}
	}

	void Clear::Bind() const
	{
		switch (m_State)
		{
			case State::ClearColorDepthStencil:
				RenderCommand::Clear(RendererAPI::ClearType::Color_Depth_Stencil);
				break;
			case State::ClearDepth:
				RenderCommand::Clear(RendererAPI::ClearType::Depth);
				break;
		}
	}
}
