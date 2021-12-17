#pragma once

#include "Engine/Renderer/RendererAPI.h"
#include "Engine/Renderer/Renderer.h"

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
		static void DrawInstanced(uint32_t instanceCount)
		{
			s_RendererAPI->DrawInstanced(instanceCount);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0, int32_t baseIndex = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount, baseIndex);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawIndexed(int32_t baseVertex, uint32_t indexCount = 0, int32_t baseIndex = 0)
		{
			s_RendererAPI->DrawIndexed(baseVertex, indexCount, baseIndex);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawLine(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLine(vertexArray, vertexCount);
		}

		/** Issue a draw call. */
		static void DrawLineIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawLineIndexed(vertexArray, indexCount);
		}

		static void SetLineThickness(float thickness)
		{
			s_RendererAPI->SetLineThickness(thickness);
		}

		static void ToggleFaceCulling(bool bEnable)
		{
			s_RendererAPI->ToggleFaceCulling(bEnable);
		}

		static void ToggleDepthTesting(bool bEnable)
		{
			s_RendererAPI->ToggleDepthTesting(bEnable);
		}

		static void ToggleDepthWriting(bool bEnable)
		{
			s_RendererAPI->ToggleDepthWriting(bEnable);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
