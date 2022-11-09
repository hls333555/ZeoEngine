#include "ZEpch.h"
#include "Engine/Renderer/RenderPassInput.h"

namespace ZeoEngine {

	RenderPassInput::RenderPassInput(std::string name)
		: m_Name(std::move(name))
	{
		if (m_Name.empty())
		{
			ZE_CORE_ASSERT(false, "Render pass input name should not be empty!");
		}
	}

	void RenderPassInput::PostLinkValidate() const
	{
		if (!m_HasLinked)
		{
			ZE_CORE_ERROR("Unlinked render pass input: {0}!", GetName());
		}
	}

	void RenderPassInput::SetLinkTarget(std::string targetPass, std::string targetOutput)
	{
		if (targetPass.empty())
		{
			ZE_CORE_ASSERT(false, "Failed to link target! Target render pass name should not be empty.");
		}
		m_TargetPassName = std::move(targetPass);

		if (targetOutput.empty())
		{
			ZE_CORE_ASSERT(false, "Failed to link target! Target output name should not be empty.");
		}
		m_TargetOutputName = std::move(targetOutput);
	}

}
