#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine {

	class VertexArray;
	class Shader;

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
		};

	public:
		virtual void Init() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0, int32_t baseIndex = 0) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, int32_t baseVertex, uint32_t indexCount = 0, int32_t baseIndex = 0) = 0;
		virtual void DrawInstanced(uint32_t instanceCount) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void SetLineThickness(float thickness) = 0;

		virtual void ToggleFaceCulling(bool bEnable) = 0;
		virtual void EnableDepthWriting(bool bEnable) = 0;

		static API GetAPI() { return s_API; }
		static bool Is2D() { return s_bIs2D; }

		static Scope<RendererAPI> Create();

	private:
		static API s_API;
		static bool s_bIs2D;

	};

}
