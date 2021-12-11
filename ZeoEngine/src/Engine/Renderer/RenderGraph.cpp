#include "ZEpch.h"
#include "Engine/Renderer/RenderGraph.h"

#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/RenderPassSink.h"
#include "Engine/Renderer/RenderPassSource.h"
#include "Engine/Renderer/RenderStep.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	RenderGraph::RenderGraph(const Ref<FrameBuffer>& fbo)
		: m_BackFBO(fbo)
	{
		AddGlobalSink(RenderPassBufferSink<FrameBuffer>::Create("BackFrameBuffer", m_BackFBO));
		AddGlobalSource(RenderPassBufferSource<FrameBuffer>::Create("BackFrameBuffer", m_BackFBO));
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

	void RenderGraph::AddGlobalSink(Scope<RenderPassSink> sink)
	{
		m_GlobalSinks.emplace_back(std::move(sink));
	}

	void RenderGraph::AddGlobalSource(Scope<RenderPassSource> source)
	{
		m_GlobalSources.emplace_back(std::move(source));
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

		LinkSinks(pass);
		m_Passes.emplace_back(std::move(pass));
	}

	void RenderGraph::SetGlobalSinkLinkage(const std::string& sinkName, const std::string& targetName)
	{
		const auto it = std::find_if(m_GlobalSinks.begin(), m_GlobalSinks.end(), [&sinkName](const std::unique_ptr<RenderPassSink>& sink)
		{
			return sink->GetName() == sinkName;
		});
		if (it == m_GlobalSinks.end())
		{
			ZE_CORE_ERROR("Global render pass sink {0} does not exist!", sinkName);
		}

		auto targetSplit = EngineUtils::SplitString(targetName, '.');
		if (targetSplit.size() != 2)
		{
			ZE_CORE_ERROR("Failed to link sink target with incorrect target name format \"{0}\"!", targetName);
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
		LinkGlobalSinks();
		m_bFinalized = true;
	}

	void RenderGraph::LinkSinks(const Scope<RenderPass>& pass)
	{
		for (const auto& sink : pass->GetSinks())
		{
			const auto& inputPassName = sink->GetTargetRenderPassName();

			if (inputPassName == "$")
			{
				bool bHasBound = false;
				for (const auto& source : m_GlobalSources)
				{
					if (source->GetName() == sink->GetTargetOutputName())
					{
						sink->Bind(source.get());
						bHasBound = true;
						break;
					}
				}
				if (!bHasBound)
				{
					ZE_CORE_ERROR("Failed to link render pass sink {0} to global sources!", sink->GetTargetOutputName());
				}
			}
			else
			{
				for (const auto& pass : m_Passes)
				{
					if (pass->GetName() == inputPassName)
					{
						auto* source = pass->GetSource(sink->GetTargetOutputName());
						sink->Bind(source);
						break;
					}
				}
			}
		}
	}

	void RenderGraph::LinkGlobalSinks()
	{
		for (const auto& sink : m_GlobalSinks)
		{
			const auto& inputPassName = sink->GetTargetRenderPassName();
			for (const auto& pass : m_Passes)
			{
				if (pass->GetName() == inputPassName)
				{
					auto* source = pass->GetSource(sink->GetTargetOutputName());
					sink->Bind(source);
					break;
				}
			}
		}
	}

	ForwardRenderGraph::ForwardRenderGraph(const Ref<FrameBuffer>& fbo)
		: RenderGraph(fbo)
	{
		{
			auto pass = CreateScope<FrameBufferClearPass>("ClearFrameBuffer");
			pass->SetSinkLinkage("FrameBuffer", "$.BackFrameBuffer");
			AddRenderPass(std::move(pass));
		}
		{
			auto pass = CreateScope<OpaqueRenderPass>("Opaque");
			pass->SetSinkLinkage("FrameBuffer", "ClearFrameBuffer.FrameBuffer");
			AddRenderPass(std::move(pass));
		}
		SetGlobalSinkLinkage("BackFrameBuffer", "Opaque.FrameBuffer");
		Finalize();
	}

}
