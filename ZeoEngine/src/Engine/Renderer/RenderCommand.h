#pragma once

#include "RendererAPI.h"

namespace ZeoEngine {

	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		/** Call this before any rendering calls! */
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		/** Issue a draw call. */
		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};

}
