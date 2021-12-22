#pragma once

#include "Engine/Renderer/RendererAPI.h"

namespace ZeoEngine {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear(ClearType type) override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0, int32_t baseIndex = 0) override;
		virtual void DrawIndexed(int32_t baseVertex, uint32_t indexCount = 0, int32_t baseIndex = 0) override;
		virtual void DrawInstanced(uint32_t instanceCount) override;
		virtual void DrawLine(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void DrawLineIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void SetLineThickness(float thickness) override;

		virtual void ToggleFaceCulling(bool bEnable) override;
		virtual void SetFaceCullingMode(bool bIsBack) override;
		virtual void ToggleDepthTesting(bool bEnable) override;
		virtual void ToggleDepthWriting(bool bEnable) override;

	};

}
