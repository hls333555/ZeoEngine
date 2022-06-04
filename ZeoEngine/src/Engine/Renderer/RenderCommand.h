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

		static void SetViewport(U32 x, U32 y, U32 width, U32 height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const Vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		/** Call this before any rendering calls! */
		static void Clear(RendererAPI::ClearType type)
		{
			s_RendererAPI->Clear(type);
		}

		/** Issue a draw call. */
		static void DrawArrays(U32 vertexCount)
		{
			s_RendererAPI->DrawArrays(vertexCount);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawInstanced(U32 instanceCount)
		{
			s_RendererAPI->DrawInstanced(instanceCount);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount = 0, I32 baseIndex = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount, baseIndex);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawIndexed(I32 baseVertex, U32 indexCount = 0, I32 baseIndex = 0)
		{
			s_RendererAPI->DrawIndexed(baseVertex, indexCount, baseIndex);
			++Renderer::GetStats().DrawCalls;
		}

		/** Issue a draw call. */
		static void DrawLine(const Ref<VertexArray>& vertexArray, U32 vertexCount)
		{
			s_RendererAPI->DrawLine(vertexArray, vertexCount);
		}

		/** Issue a draw call. */
		static void DrawLineIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount = 0)
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
