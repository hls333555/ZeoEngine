#pragma once

#include "Engine/GameFramework/Entity.h"
#include "Engine/Renderer/RendererAPI.h"

namespace ZeoEngine {

	class RenderGraph;

	struct RendererStats
	{
		uint32_t DrawCalls = 0;
		uint32_t QuadCount = 0;
		uint32_t MeshVertexCount = 0;

		Entity HoveredEntity;

		uint32_t GetTotalVertexCount() const { return QuadCount * 4 + MeshVertexCount; }

		void Reset()
		{
			DrawCalls = 0;
			QuadCount = 0;
			MeshVertexCount = 0;
			HoveredEntity = {};
		}
	};

	struct RendererData
	{
		RenderGraph* ActiveRenderGraph = nullptr;
		RendererStats Stats;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static RenderGraph* GetActiveRenderGraph() { return s_Data.ActiveRenderGraph; }
		static void SetActiveRenderGraph(RenderGraph* renderGraph) { s_Data.ActiveRenderGraph = renderGraph; }

		static void OnWindowResize(uint32_t width, uint32_t height);

		static RendererStats& GetStats();
		static void ResetStats();

	private:
		static RendererData s_Data;
	};

}
