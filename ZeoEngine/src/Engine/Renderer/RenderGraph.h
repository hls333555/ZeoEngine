#pragma once

namespace ZeoEngine {

	class RenderPass;
	class RenderQueuePass;
	class RenderPassSink;
	class RenderPassSource;
	class FrameBuffer;

	class RenderGraph
	{
	public:
		explicit RenderGraph(const Ref<FrameBuffer>& fbo);
		~RenderGraph();

		void Execute();
		void Reset();
		RenderQueuePass* GetRenderQueuePass(const std::string& passName);

	protected:
		void AddGlobalSink(Scope<RenderPassSink> sink);
		void AddGlobalSource(Scope<RenderPassSource> source);
		void AddRenderPass(Scope<RenderPass> pass);
		void SetGlobalSinkLinkage(const std::string& sinkName, const std::string& targetName);
		void Finalize();

	private:
		/** Link outputs from passes (and global outputs) to pass inputs. */
		void LinkSinks(const Scope<RenderPass>& pass);
		void LinkGlobalSinks();

	private:
		std::vector<Scope<RenderPass>> m_Passes;
		std::vector<Scope<RenderPassSink>> m_GlobalSinks;
		std::vector<Scope<RenderPassSource>> m_GlobalSources;
		Ref<FrameBuffer> m_BackFBO;
		bool m_bFinalized = false;
	};

	class ForwardRenderGraph : public RenderGraph
	{
	public:
		explicit ForwardRenderGraph(const Ref<FrameBuffer>& fbo);
	};

}
