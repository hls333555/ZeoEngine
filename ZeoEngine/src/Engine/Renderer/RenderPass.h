#pragma once

#include "Engine/Renderer/RenderTask.h"
#include "Engine/Renderer/RenderPassSink.h"
#include "Engine/Renderer/RenderPassSource.h"
#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class FrameBuffer;

	class RenderPass
	{
	public:
		explicit RenderPass(std::string name);
		virtual ~RenderPass() = default;

		const std::string& GetName() const { return m_Name; }
		const auto& GetSinks() const { return m_Sinks; }
		RenderPassSink* GetSink(const std::string& name) const;
		RenderPassSource* GetSource(const std::string& name) const;

		virtual void Execute() const = 0;
		virtual void Reset() {}
		virtual void Finalize();

		void SetSinkLinkage(const std::string& sinkName, const std::string& targetName);

	protected:
		void RegisterSink(Scope<RenderPassSink> sink);
		void RegisterSource(Scope<RenderPassSource> source);

	private:
		std::string m_Name;
		std::vector<Scope<RenderPassSink>> m_Sinks;
		std::vector<Scope<RenderPassSource>> m_Sources;
	};

	class FrameBufferClearPass : public RenderPass
	{
	public:
		explicit FrameBufferClearPass(std::string name);

		virtual void Execute() const override;

	private:
		Ref<FrameBuffer> m_FBO;
	};

	class BindingPass : public RenderPass
	{
	public:
		using RenderPass::RenderPass;

	protected:
		void AddBind(Ref<Bindable> bindable);
		void BindAll() const;
		virtual void Finalize() override;

	private:
		void BindBufferResource() const;

	protected:
		Ref<FrameBuffer> m_FBO;
	private:
		std::vector<Ref<Bindable>> m_Bindables;
	};

	class RenderQueuePass : public BindingPass
	{
	public:
		using BindingPass::BindingPass;

		void AddTask(RenderTask task);
		virtual void Execute() const override;
		virtual void Reset() override;

	private:
		std::vector<RenderTask> m_Tasks;
	};

	class OpaqueRenderPass : public RenderQueuePass
	{
	public:
		explicit OpaqueRenderPass(std::string name);
	};

}
