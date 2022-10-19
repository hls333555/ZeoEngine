#pragma once

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class RenderPass;
	class BindingPass;
	class RenderQueuePass;
	class RenderPassInput;
	class RenderPassOutput;
	class FrameBuffer;

	class RenderGraph
	{
	public:
		RenderGraph();
		virtual ~RenderGraph();

		Ref<FrameBuffer> GetBackFrameBuffer() const { return m_BackFBO; }

		void Init();
		void Start() const;
		void Execute() const;
		void Stop() const;

		void OnViewportResize(U32 width, U32 height) const;

		void ToggleRenderPassActive(const std::string& passName, bool bActive);

		template<typename T>
		T* GetRenderPass(const std::string& passName) const
		{
			for (const auto& pass : m_Passes)
			{
				if (pass->GetName() == passName)
				{
					T* outPass = dynamic_cast<T*>(pass.get());
					if (!outPass)
					{
						ZE_CORE_ERROR("Failed to get render pass {0} with invalid type!", passName);
					}
					return outPass;
				}
			}

			ZE_CORE_ERROR("Failed to find render pass with unknown name: {0}!", passName);
			return nullptr;
		}

	protected:
		constexpr const char* GetBackFrameBufferName() const { return "BackFrameBuffer"; }
		void AddGlobalInput(Scope<RenderPassInput> input);
		void AddGlobalOutput(Scope<RenderPassOutput> output);
		void AddRenderPass(Scope<RenderPass> pass);
		void SetGlobalInputLinkage(const std::string& inputName, const std::string& targetOutputName);
		void Finalize();

	private:
		virtual Ref<FrameBuffer> CreateBackFrameBuffer() = 0;
		virtual void InitRenderPasses() = 0;

		/** Link pass inputs to outputs from passes (and global outputs). */
		void LinkInputs(const Scope<RenderPass>& pass) const;
		void LinkGlobalInputs() const;
		void Reset() const;

	private:
		std::vector<Scope<RenderPass>> m_Passes;
		std::vector<Scope<RenderPassInput>> m_GlobalInputs;
		std::vector<Scope<RenderPassOutput>> m_GlobalOutputs;
		Ref<FrameBuffer> m_BackFBO;
		bool m_bFinalized = false;
	};

	class ForwardRenderGraph : public RenderGraph
	{
	private:
		virtual Ref<FrameBuffer> CreateBackFrameBuffer() override;
		virtual void InitRenderPasses() override;
	};

	class AssetPreviewRenderGraph : public RenderGraph
	{
	private:
		virtual Ref<FrameBuffer> CreateBackFrameBuffer() override;
		virtual void InitRenderPasses() override;
	};

}
