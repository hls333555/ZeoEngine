#pragma once

#include "Engine/Renderer/RendererAPI.h"

namespace ZeoEngine {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;

		virtual void SetViewport(U32 x, U32 y, U32 width, U32 height) override;

		virtual void SetClearColor(const Vec4& color) override;
		virtual void Clear(ClearType type) override;

		virtual void DrawArrays(U32 vertexCount) override;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount = 0, I32 baseIndex = 0) override;
		virtual void DrawIndexed(I32 baseVertex, U32 indexCount = 0, I32 baseIndex = 0) override;
		virtual void DrawInstanced(U32 instanceCount) override;
		virtual void DrawLine(const Ref<VertexArray>& vertexArray, U32 vertexCount) override;
		virtual void DrawLineIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount = 0) override;
		virtual void SetLineThickness(float thickness) override;

		virtual void DispatchCompute(U32 x, U32 y, U32 z) override;
		virtual void SetImageAccessBarrier() override;

		virtual void ToggleBlend(bool bEnable) override;
		virtual void ToggleCullFace(bool bEnable) override;
		virtual void SetCullFaceMode(bool bIsBack) override;
		virtual void ToggleDepthTest(bool bEnable) override;
		virtual void ToggleDepthWrite(bool bEnable) override;
		virtual void ToggleDepthClamp(bool bEnable) override;

	};

}
