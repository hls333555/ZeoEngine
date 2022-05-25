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
		static void Clear(RendererAPI::ClearType type)
		{
			s_RendererAPI->Clear(type);
		}

		/** Issue a draw call. */
		static void DrawArrays(uint32_t vertexCount)
		{
			s_RendererAPI->DrawArrays(vertexCount);
			++Renderer::GetStats().DrawCalls;
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

		static void ToggleBlend(bool bEnable)
		{
			s_RendererAPI->ToggleBlend(bEnable);
		}

		static void ToggleCullFace(bool bEnable)
		{
			s_RendererAPI->ToggleCullFace(bEnable);
		}

		static void SetCullFaceMode(bool bIsBack)
		{
			s_RendererAPI->SetCullFaceMode(bIsBack);
		}

		static void ToggleDepthTest(bool bEnable)
		{
			s_RendererAPI->ToggleDepthTest(bEnable);
		}

		static void ToggleDepthWrite(bool bEnable)
		{
			s_RendererAPI->ToggleDepthWrite(bEnable);
		}

		static void ToggleDepthClamp(bool bEnable)
		{
			s_RendererAPI->ToggleDepthClamp(bEnable);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
