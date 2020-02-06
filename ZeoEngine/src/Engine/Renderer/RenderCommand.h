#pragma once

#include "Engine/Renderer/RendererAPI.h"

namespace ZeoEngine {

	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		/** Call this before any rendering calls! */
		static void Clear()
		{
			s_RendererAPI->Clear();
		}

		/** Issue a draw call. */
		static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		/** Should be called before and after drawing translucent objects. */
		static void EnableDepthWriting(bool bEnable)
		{
			s_RendererAPI->EnableDepthWriting(bEnable);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
