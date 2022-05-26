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

		virtual void Execute() const = 0;
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
		explicit BindingPass(std::string name, bool bAutoActive = true);

	protected:
		void AddBindable(Ref<Bindable> bindable);
		void BindAll() const;
		void UnbindBindables() const;
		virtual void Finalize() override;

		/** Register a bindable input and add that to the bindable list. */
		template<class T>
		void RegisterBindableInput(std::string name)
		{
			const auto index = m_Bindables.size();
			m_Bindables.emplace_back();
			RegisterInput(RenderPassContainerBindableInput<T>::Create(std::move(name), m_Bindables, index));
		}

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
		void ExecuteTasks() const;
		virtual void Execute() const override;
		virtual void Reset() override;
	private:
		virtual void ClearFrameBufferAttachment() const {}

	private:
		std::vector<RenderTask> m_Tasks;
	};

	class FullscreenPass : public BindingPass
	{
	public:
		using BindingPass::BindingPass;

		virtual void Execute() const override;
	};

	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		explicit ShadowMappingPass(std::string name, bool bAutoActive = true);

		static Ref<FrameBuffer> GetDepthFrameBuffer() { return s_FBO; }

	private:
		void CreateDepthBuffer();

	private:
		static Ref<FrameBuffer> s_FBO;
	};

	class ScreenSpaceShadowPass : public RenderQueuePass
	{
	public:
		explicit ScreenSpaceShadowPass(std::string name, bool bAutoActive = true);

		static Ref<FrameBuffer> GetShadowFrameBuffer() { return s_FBO; }

	private:
		void CreateShadowBuffer();

	private:
		static Ref<FrameBuffer> s_FBO;
	};

	class HorizontalBlurPass : public FullscreenPass
	{
	public:
		explicit HorizontalBlurPass(std::string name, bool bAutoActive = true);

		static Ref<FrameBuffer> GetHorizontalBlurFrameBuffer() { return s_FBO; }

	private:
		void CreateHorizontalBlurBuffer();

	private:
		static Ref<FrameBuffer> s_FBO;
	};

	class VerticalBlurPass : public FullscreenPass
	{
	public:
		explicit VerticalBlurPass(std::string name, bool bAutoActive = true);

		static Ref<FrameBuffer> GetVerticalBlurFrameBuffer() { return s_FBO; }

	private:
		void CreateVerticalBlurBuffer();

	private:
		static Ref<FrameBuffer> s_FBO;
	};

	class OpaqueRenderPass : public RenderQueuePass
	{
	public:
		explicit OpaqueRenderPass(std::string name, bool bShouldClearIDBuffer, bool bAutoActive = true);

	private:
		virtual void ClearFrameBufferAttachment() const override;

	private:
		bool m_bShouldClearIDBuffer;
	};

	class GridRenderPass : public BindingPass
	{
	public:
		explicit GridRenderPass(std::string name, bool bAutoActive = true);

		virtual void Execute() const override;
	};

}
