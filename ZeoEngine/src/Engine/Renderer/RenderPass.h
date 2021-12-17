#pragma once

#include "Engine/Renderer/RenderTask.h"
#include "Engine/Renderer/RenderPassInput.h"
#include "Engine/Renderer/RenderPassOutput.h"
#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class Shader;

	class RenderPass
	{
	public:
		explicit RenderPass(std::string name, bool bAutoActive = true);
		virtual ~RenderPass() = default;

		const std::string& GetName() const { return m_Name; }
		bool IsActive() const { return m_bIsActive; }
		void ToggleActive(bool bActive) { m_bIsActive = bActive; }
		const auto& GetInputs() const { return m_Inputs; }
		RenderPassInput* GetInput(const std::string& name) const;
		RenderPassOutput* GetOuput(const std::string& name) const;

		virtual bool Execute() const = 0;
		virtual void Reset() {}
		virtual void Finalize();

		void SetInputLinkage(const std::string& inputName, const std::string& targetOutputName);

	protected:
		void RegisterInput(Scope<RenderPassInput> input);
		void RegisterOutput(Scope<RenderPassOutput> output);

	private:
		std::string m_Name;
		bool m_bIsActive = true;
		std::vector<Scope<RenderPassInput>> m_Inputs;
		std::vector<Scope<RenderPassOutput>> m_Outputs;
	};

	class BindingPass : public RenderPass
	{
	public:
		using RenderPass::RenderPass;

	protected:
		void AddBindable(Ref<Bindable> bindable);
		void BindAll() const;
		virtual bool Execute() const override;
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
		virtual bool Execute() const override;
		virtual void Reset() override;

	private:
		std::vector<RenderTask> m_Tasks;
	};

	class OpaqueRenderPass : public RenderQueuePass
	{
	public:
		explicit OpaqueRenderPass(std::string name, bool bAutoActive = true);
	};

	class GridRenderPass : public BindingPass
	{
	public:
		explicit GridRenderPass(std::string name, bool bAutoActive = true);

		virtual bool Execute() const override;
	};

}
