#include "ZEpch.h"
#include "Engine/Renderer/BindableStates.h"

#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	void Depth::Bind() const
	{
		switch (m_State)
		{
			case State::ReadWrite:
				RenderCommand::ToggleDepthTest(true);
				RenderCommand::ToggleDepthWrite(true);
				break;
			case State::ReadOnly:
				RenderCommand::ToggleDepthTest(true);
				RenderCommand::ToggleDepthWrite(false);
				break;
			case State::Disable:
				RenderCommand::ToggleDepthTest(false);
				break;
		}
	}

	void DepthClamp::Bind() const
	{
		switch (m_State)
		{
			case State::Disable:
				RenderCommand::ToggleDepthClamp(false);
				break;
			case State::Enable:
				RenderCommand::ToggleDepthClamp(true);
				break;
		}
	}

	void Blend::Bind() const
	{
		switch (m_State)
		{
			case State::Enable:
				RenderCommand::ToggleBlend(true);
				break;
			case State::Disable:
				RenderCommand::ToggleBlend(false);
				break;
		}
	}

	void TwoSided::Bind() const
	{
		switch (m_State)
		{
			case State::CullBack:
				RenderCommand::ToggleCullFace(true);
				RenderCommand::SetCullFaceMode(true);
				break;
			case State::CullFront:
				RenderCommand::ToggleCullFace(true);
				RenderCommand::SetCullFaceMode(false);
				break;
			case State::Disable:
				RenderCommand::ToggleCullFace(false);
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
