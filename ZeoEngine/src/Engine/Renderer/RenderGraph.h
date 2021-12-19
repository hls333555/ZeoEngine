#pragma once

namespace ZeoEngine {

	class RenderPass;
	class RenderQueuePass;
	class RenderPassInput;
	class RenderPassOutput;
	class FrameBuffer;

	class RenderGraph
	{
	public:
		explicit RenderGraph(const Ref<FrameBuffer>& fbo);
		~RenderGraph();

		void Execute();
		void Reset();
		void ToggleRenderPassActive(const std::string& passName, bool bActive);
		RenderQueuePass* GetRenderQueuePass(const std::string& passName) const;

	protected:
		void AddGlobalInput(Scope<RenderPassInput> input);
		void AddGlobalOutput(Scope<RenderPassOutput> output);
		void AddRenderPass(Scope<RenderPass> pass);
		void SetGlobalInputLinkage(const std::string& inputName, const std::string& targetOutputName);
		void Finalize();

	private:
		/** Link pass inputs to outputs from passes (and global outputs). */
		void LinkInputs(const Scope<RenderPass>& pass);
		void LinkGlobalInputs();

	private:
		std::vector<Scope<RenderPass>> m_Passes;
		std::vector<Scope<RenderPassInput>> m_GlobalInputs;
		std::vector<Scope<RenderPassOutput>> m_GlobalOutputs;
		Ref<FrameBuffer> m_BackFBO;
		bool m_bFinalized = false;
	};

	class ForwardRenderGraph : public RenderGraph
	{
	public:
		explicit ForwardRenderGraph(const Ref<FrameBuffer>& fbo, bool bDrawGrid = false);
	};

}
