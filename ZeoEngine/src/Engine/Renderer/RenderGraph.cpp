#include "ZEpch.h"
#include "Engine/Renderer/RenderGraph.h"

#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RenderPassInput.h"
#include "Engine/Renderer/RenderPassOutput.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	RenderGraph::RenderGraph()
	{
		AddGlobalInput(RenderPassBufferInput<FrameBuffer>::Create(GetBackFrameBufferName(), m_BackFBO));
		AddGlobalOutput(RenderPassBufferOutput<FrameBuffer>::Create(GetBackFrameBufferName(), m_BackFBO));
	}

	RenderGraph::~RenderGraph() = default;

	void RenderGraph::Init()
	{
		m_BackFBO = CreateBackFrameBuffer();
		InitRenderPasses();
	}

	void RenderGraph::Start() const
	{
		m_BackFBO->BindAsBuffer();
	}

	void RenderGraph::Execute() const
	{
		ZE_CORE_ASSERT(m_bFinalized);

		for (const auto& pass : m_Passes)
		{
			pass->Execute();
		}
	}

	void RenderGraph::Stop() const
	{
		Reset();
		m_BackFBO->UnbindAsBuffer();
	}

	void RenderGraph::Reset() const
	{
		ZE_CORE_ASSERT(m_bFinalized);

		for (const auto& pass : m_Passes)
		{
			pass->Reset();
		}
	}

	RenderQueuePass* RenderGraph::GetRenderQueuePass(const std::string& passName) const
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

	void RenderGraph::ToggleRenderPassActive(const std::string& passName, bool bActive)
	{
		for (const auto& pass : m_Passes)
		{
			if (pass->GetName() == passName)
			{
				pass->ToggleActive(bActive);
				return;
			}
		}
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

	void RenderGraph::LinkInputs(const Scope<RenderPass>& pass) const
	{
		for (const auto& input : pass->GetInputs())
		{
			const auto& inputPassName = input->GetTargetRenderPassName();
			const auto& inputOutputName = input->GetTargetOutputName();

			if (inputPassName == "$")
			{
				bool bHasBound = false;
				for (const auto& output : m_GlobalOutputs)
				{
					if (output->GetName() == inputOutputName)
					{
						input->Bind(output.get());
						bHasBound = true;
						break;
					}
				}
				if (!bHasBound)
				{
					ZE_CORE_ERROR("Failed to link render pass input {0} to global outputs!", inputOutputName);
				}
			}
			else
			{
				bool bHasBound = false;
				for (const auto& p : m_Passes)
				{
					if (p->GetName() == inputPassName)
					{
						const auto* output = p->GetOuput(inputOutputName);
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

	void RenderGraph::LinkGlobalInputs() const
	{
		for (const auto& input : m_GlobalInputs)
		{
			const auto& inputPassName = input->GetTargetRenderPassName();
			for (const auto& pass : m_Passes)
			{
				if (pass->GetName() == inputPassName)
				{
					const auto* output = pass->GetOuput(input->GetTargetOutputName());
					input->Bind(output);
					break;
				}
			}
		}
	}

	Ref<FrameBuffer> ForwardRenderGraph::CreateBackFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::RGBA16F, { SamplerType::BilinearClamp } }, // Entity ID buffer
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
		};
		return FrameBuffer::Create(fbSpec);
	}

	void ForwardRenderGraph::InitRenderPasses()
	{
		{
			auto pass = CreateScope<ShadowMappingPass>("ShadowMapping");
			AddRenderPass(std::move(pass));
		}
		{
			auto pass = CreateScope<ScreenSpaceShadowPass>("ScreenSpaceShadow");
			pass->SetInputLinkage("ShadowMap", "ShadowMapping.ShadowMap");
			AddRenderPass(std::move(pass));
		}
		//{
		//	auto pass = CreateScope<HorizontalBlurPass>("HorizontalBlur");
		//	pass->SetInputLinkage("ShadowMap", "ScreenSpaceShadow.ShadowMap");
		//	AddRenderPass(std::move(pass));
		//}
		//{
		//	auto pass = CreateScope<VerticalBlurPass>("VerticalBlur");
		//	pass->SetInputLinkage("ShadowMap", "HorizontalBlur.ShadowMap");
		//	AddRenderPass(std::move(pass));
		//}
		{
			auto pass = CreateScope<OpaqueRenderPass>("Opaque", true);
			pass->SetInputLinkage("ShadowMap", "ScreenSpaceShadow.ShadowMap");
			pass->SetInputLinkage("FrameBuffer", "$.BackFrameBuffer");
			AddRenderPass(std::move(pass));
		}
		{
			auto pass = CreateScope<GridRenderPass>("Grid");
			pass->SetInputLinkage("FrameBuffer", "Opaque.FrameBuffer");
			AddRenderPass(std::move(pass));
		}
		SetGlobalInputLinkage(GetBackFrameBufferName(), "Grid.FrameBuffer");
		Finalize();
	}

	Ref<FrameBuffer> EditorPreviewRenderGraph::CreateBackFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
		};
		return FrameBuffer::Create(fbSpec);
	}

	// TODO:
	void EditorPreviewRenderGraph::InitRenderPasses()
	{
		{
			auto pass = CreateScope<ShadowMappingPass>("ShadowMapping");
			AddRenderPass(std::move(pass));
		}
		{
			auto pass = CreateScope<ScreenSpaceShadowPass>("ScreenSpaceShadow");
			pass->SetInputLinkage("ShadowMap", "ShadowMapping.ShadowMap");
			AddRenderPass(std::move(pass));
		}
		{
			auto pass = CreateScope<OpaqueRenderPass>("Opaque", false);
			pass->SetInputLinkage("ShadowMap", "ScreenSpaceShadow.ShadowMap");
			pass->SetInputLinkage("FrameBuffer", "$.BackFrameBuffer");
			AddRenderPass(std::move(pass));
		}
		SetGlobalInputLinkage(GetBackFrameBufferName(), "Opaque.FrameBuffer");
		Finalize();
	}

}
