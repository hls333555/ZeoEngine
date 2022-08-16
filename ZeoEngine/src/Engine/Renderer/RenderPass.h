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
		virtual void OnViewportResize(U32 width, U32 height) const = 0;

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

	struct ComputeResource
	{
		U32 BindingSlot;

		explicit ComputeResource(U32 bindingSlot) : BindingSlot(bindingSlot) {}
		virtual ~ComputeResource() = default;

		virtual Ref<Texture> GetTexture() const = 0;
	};

	struct ComputeBufferResource : public ComputeResource
	{
		const Ref<FrameBuffer>& FrameBufferResource;
		bool bIsColorAttachment;
		U32 ColorAttachmentIndex;

		ComputeBufferResource(U32 bindingSlot, const Ref<FrameBuffer>& fbo, bool bIsColorAttachment, U32 colorAttachmentIndex = 0)
			: ComputeResource(bindingSlot)
			, FrameBufferResource(fbo), bIsColorAttachment(bIsColorAttachment), ColorAttachmentIndex(colorAttachmentIndex) {}

		virtual Ref<Texture> GetTexture() const override { return bIsColorAttachment ? FrameBufferResource->GetColorAttachment(ColorAttachmentIndex) : FrameBufferResource->GetDepthAttachment(); }
	};

	struct ComputeTextureResource : public ComputeResource
	{
		Ref<Texture> TextureResource;

		ComputeTextureResource(U32 bindingSlot, const Ref<Texture>& texture)
			: ComputeResource(bindingSlot)
			, TextureResource(texture) {}

		virtual Ref<Texture> GetTexture() const override { return TextureResource; }
	};

	struct ComputeStage
	{
		Scope<ComputeResource> TextureToRead;
		Scope<ComputeResource> TextureToWrite;
		UVec3 DispatchParams;
	};

	class ComputePass : public RenderPass
	{
	public:
		using RenderPass::RenderPass;

		void SetComputeShader(Ref<Shader> shader) { m_ComputeShader = std::move(shader); }

		virtual void Execute() const override;

	protected:
		void AddComputeStage(ComputeStage stage);
		void BindComputeShader() const;
		void UnbindComputeShader() const;

	private:
		Ref<Shader> m_ComputeShader;
		std::vector<ComputeStage> m_ComputeStages;
	};

	class BindingPass : public RenderPass
	{
	public:
		explicit BindingPass(std::string name, bool bAutoActive = true);

		virtual void Finalize() override;

		virtual void OnViewportResize(U32 width, U32 height) const override;

	protected:
		void AddBindable(Ref<Bindable> bindable);
		void BindAll() const;
		void BindBindables() const;
		void UnbindBindables() const;
		Ref<Bindable> GetBindableByIndex(U32 index);

		/** Register a bindable input and add that to the bindable list. */
		template<class T>
		SizeT RegisterBindableInput(std::string name)
		{
			const auto index = m_Bindables.size();
			auto bindable = m_Bindables.emplace_back();
			RegisterInput(RenderPassContainerBindableInput<T>::Create(std::move(name), m_Bindables, index));
			return index;
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

		void AddTask(const Drawable* drawable, const RenderStep* step);
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
		Ref<class ShaderInstance> GetShadowShader() const { return m_ShadowShader; }

	private:
		void CreateShadowBuffer();

	private:
		static Ref<FrameBuffer> s_FBO; // TODO: Remove or change to non-static one
		Ref<ShaderInstance> m_ShadowShader;
	};

	class OpaqueRenderPass : public RenderQueuePass
	{
	public:
		explicit OpaqueRenderPass(std::string name, bool bShouldClearIDBuffer, bool bAutoActive = true);

	private:
		virtual void ClearFrameBufferAttachment() const override;

		void CreateFrameBuffer();

	private:
		bool m_bShouldClearIDBuffer;
	};

	class GridRenderPass : public BindingPass
	{
	public:
		explicit GridRenderPass(std::string name, bool bAutoActive = true);

		virtual void Execute() const override;
	};

	class PostProcessingPass : public FullscreenPass
	{
	public:
		explicit PostProcessingPass(std::string name, bool bAutoActive = true);

		virtual void Execute() const override;
		virtual void Finalize() override;

	private:
		SizeT m_SceneBufferIndex;
		Ref<FrameBuffer> m_SceneBuffer;
	};

}
