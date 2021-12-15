#include "ZEpch.h"
#include "Engine/Renderer/RenderGraph.h"

#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RenderPassInput.h"
#include "Engine/Renderer/RenderPassOutput.h"
#include "Engine/Renderer/RenderStep.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	RenderGraph::RenderGraph(const Ref<FrameBuffer>& fbo)
		: m_BackFBO(fbo)
	{
		AddGlobalInput(RenderPassBufferInput<FrameBuffer>::Create("BackFrameBuffer", m_BackFBO));
		AddGlobalOutput(RenderPassBufferOutput<FrameBuffer>::Create("BackFrameBuffer", m_BackFBO));
	}

	RenderGraph::~RenderGraph() = default;

	void RenderGraph::Execute()
	{
		ZE_CORE_ASSERT(m_bFinalized);

		for (const auto& pass : m_Passes)
		{
			pass->Execute();
		}
	}

	void RenderGraph::Reset()
	{
		ZE_CORE_ASSERT(m_bFinalized);

		for (const auto& pass : m_Passes)
		{
			pass->Reset();
		}
	}

	RenderQueuePass* RenderGraph::GetRenderQueuePass(const std::string& passName)
	{
		for (const auto& pass : m_Passes)
		{
			if (pass->GetName() == passName)
			{
				auto* renderQueuePass = dynamic_cast<RenderQueuePass*>(pass.get());
				if (renderQueuePass)
				{
					return renderQueuePass;
				}
			}
		}
		ZE_CORE_ERROR("Failed to find RenderQueuePass with unknown name: {0}", passName);
		return nullptr;
	}

	void RenderGraph::AddGlobalInput(Scope<RenderPassInput> input)
	{
		m_GlobalInputs.emplace_back(std::move(input));
	}

	void RenderGraph::AddGlobalOutput(Scope<RenderPassOutput> output)
	{
		m_GlobalOutputs.emplace_back(std::move(output));
	}

	void RenderGraph::AddRenderPass(Scope<RenderPass> pass)
	{
		ZE_CORE_ASSERT(!m_bFinalized);

		for (const auto& p : m_Passes)
		{
			if (p->GetName() == pass->GetName())
			{
				ZE_CORE_ERROR("Failed to add render pass {0}! Name already exists.", p->GetName());
				return;
			}
		}

		LinkInputs(pass);
		m_Passes.emplace_back(std::move(pass));
	}

	void RenderGraph::SetGlobalInputLinkage(const std::string& inputName, const std::string& targetOutputName)
	{
		const auto it = std::find_if(m_GlobalInputs.begin(), m_GlobalInputs.end(), [&inputName](const std::unique_ptr<RenderPassInput>& input)
		{
			return input->GetName() == inputName;
		});
		if (it == m_GlobalInputs.end())
		{
			ZE_CORE_ERROR("Global render pass input {0} does not exist!", inputName);
		}

		auto targetSplit = EngineUtils::SplitString(targetOutputName, '.');
		if (targetSplit.size() != 2)
		{
			ZE_CORE_ERROR("Failed to link input to output with incorrect target name format \"{0}\"!", targetOutputName);
			return;
		}

		(*it)->SetLinkTarget(std::move(targetSplit[0]), std::move(targetSplit[1]));
	}

	void RenderGraph::Finalize()
	{
		ZE_CORE_ASSERT(!m_bFinalized);

		for (const auto& pass : m_Passes)
		{
			pass->Finalize();
		}
		LinkGlobalInputs();
		m_bFinalized = true;
	}

	void RenderGraph::LinkInputs(const Scope<RenderPass>& pass)
	{
		for (const auto& input : pass->GetInputs())
		{
			const auto& inputPassName = input->GetTargetRenderPassName();

			if (inputPassName == "$")
			{
				bool bHasBound = false;
				for (const auto& output : m_GlobalOutputs)
				{
					if (output->GetName() == input->GetTargetOutputName())
					{
						input->Bind(output.get());
						bHasBound = true;
						break;
					}
				}
				if (!bHasBound)
				{
					ZE_CORE_ERROR("Failed to link render pass input {0} to global outputs!", input->GetTargetOutputName());
				}
			}
			else
			{
				bool bHasBound = false;
				for (const auto& pass : m_Passes)
				{
					if (pass->GetName() == inputPassName)
					{
						auto* output = pass->GetOuput(input->GetTargetOutputName());
						input->Bind(output);
						bHasBound = true;
						break;
					}
				}
				if (!bHasBound)
				{
					ZE_CORE_ERROR("Failed to link render pass input {0}! Target pass {1} not found.", input->GetTargetOutputName(), inputPassName);
				}
			}
		}
	}

	void RenderGraph::LinkGlobalInputs()
	{
		for (const auto& input : m_GlobalInputs)
		{
			const auto& inputPassName = input->GetTargetRenderPassName();
			for (const auto& pass : m_Passes)
			{
				if (pass->GetName() == inputPassName)
				{
					auto* output = pass->GetOuput(input->GetTargetOutputName());
					input->Bind(output);
					break;
				}
			}
		}
	}

	ForwardRenderGraph::ForwardRenderGraph(const Ref<FrameBuffer>& fbo)
		: RenderGraph(fbo)
	{
		{
			auto pass = CreateScope<OpaqueRenderPass>("Opaque");
			pass->SetInputLinkage("FrameBuffer", "$.BackFrameBuffer");
			AddRenderPass(std::move(pass));
		}
		SetGlobalInputLinkage("BackFrameBuffer", "Opaque.FrameBuffer");
		Finalize();
	}

}
