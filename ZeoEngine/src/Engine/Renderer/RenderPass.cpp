#include "ZEpch.h"
#include "Engine/Renderer/RenderPass.h"

#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	RenderPass::RenderPass(std::string name)
		: m_Name(std::move(name))
	{
	}

	RenderPassSink* RenderPass::GetSink(const std::string& name) const
	{
		for (const auto& si : m_Sinks)
		{
			if (si->GetName() == name)
			{
				return si.get();
			}
		}

		ZE_CORE_ERROR("Render pass sink {0} not found in pass {1}!", m_Name);
		return nullptr;
	}

	RenderPassSource* RenderPass::GetSource(const std::string& name) const
	{
		for (const auto& src : m_Sources)
		{
			if (src->GetName() == name)
			{
				return src.get();
			}
		}

		ZE_CORE_ERROR("Render pass source {0} not found in pass {1}!", m_Name);
		return nullptr;
	}

	void RenderPass::Finalize()
	{
		for (const auto& in : m_Sinks)
		{
			in->PostLinkValidate();
		}
		for (const auto& out : m_Sources)
		{
			out->PostLinkValidate();
		}
	}

	void RenderPass::SetSinkLinkage(const std::string& sinkName, const std::string& targetName)
	{
		auto* sink = GetSink(sinkName);
		if (!sink) return;

		auto targetSplit = EngineUtils::SplitString(targetName, '.');
		if (targetSplit.size() != 2)
		{
			ZE_CORE_ERROR("Failed to link sink target with incorrect target name format \"{0}\"!", targetName);
			return;
		}

		sink->SetLinkTarget(std::move(targetSplit[0]), std::move(targetSplit[1]));
	}

	void RenderPass::RegisterSink(Scope<RenderPassSink> sink)
	{
		for (const auto& si : m_Sinks)
		{
			if (si->GetName() == sink->GetName())
			{
				ZE_CORE_ERROR("Render pass sink {0} already registered in pass {1}!", sink->GetName(), m_Name);
				return;
			}
		}
		m_Sinks.emplace_back(std::move(sink));
	}

	void RenderPass::RegisterSource(Scope<RenderPassSource> source)
	{
		for (const auto& src : m_Sources)
		{
			if (src->GetName() == source->GetName())
			{
				ZE_CORE_ERROR("Render pass source {0} already registered in pass {1}!", source->GetName(), m_Name);
				return;
			}
		}
		m_Sources.emplace_back(std::move(source));
	}

	void BindingPass::AddBind(Ref<Bindable> bindable)
	{
		m_Bindables.emplace_back(std::move(bindable));
	}

	void BindingPass::BindAll() const
	{
		BindBufferResource();
		for (const auto& bindable : m_Bindables)
		{
			bindable->Bind();
		}
	}

	void BindingPass::Finalize()
	{
		RenderPass::Finalize();
		if (!m_FBO)
		{
			ZE_CORE_ERROR("BindingPass {0} needs a FrameBuffer!");
		}
	}

	void BindingPass::BindBufferResource() const
	{
		m_FBO->Bind();
	}

	void RenderQueuePass::AddTask(RenderTask task)
	{
		// TODO:
		m_Tasks.emplace_back(std::move(task));
	}

	void RenderQueuePass::Execute() const
	{
		BindAll();
		for (const auto& task : m_Tasks)
		{
			task.Execute();
		}
	}

	void RenderQueuePass::Reset()
	{
		m_Tasks.clear();
	}

	OpaqueRenderPass::OpaqueRenderPass(std::string name)
		: RenderQueuePass(std::move(name))
	{
		RegisterSink(RenderPassBufferSink<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterSource(RenderPassBufferSource<FrameBuffer>::Create("FrameBuffer", m_FBO));
	}

}
