#pragma once

#include "Engine/GameFramework/Entity.h"
#include "Engine/Renderer/RendererAPI.h"

namespace ZeoEngine {

	struct RendererStats
	{
		U32 DrawCalls = 0;
		U32 QuadCount = 0;
		U32 MeshVertexCount = 0;

		Entity HoveredEntity;

		U32 GetTotalVertexCount() const { return QuadCount * 4 + MeshVertexCount; }

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
		RendererStats Stats;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void OnWindowResize(U32 width, U32 height);

		static RendererStats& GetStats();
		static void ResetStats();

	private:
		static RendererData s_Data;
	};

}
