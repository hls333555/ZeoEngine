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

		enum class ClearType
		{
			Color_Depth_Stencil,
			Depth,
		};

	public:
		virtual void Init() = 0;

		virtual void SetViewport(U32 x, U32 y, U32 width, U32 height) = 0;

		virtual void SetClearColor(const Vec4& color) = 0;
		virtual void Clear(ClearType type) = 0;

		virtual void DrawArrays(U32 vertexCount) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount = 0, I32 baseIndex = 0) = 0;
		virtual void DrawIndexed(I32 baseVertex, U32 indexCount = 0, I32 baseIndex = 0) = 0;
		virtual void DrawInstanced(U32 instanceCount) = 0;
		virtual void DrawLine(const Ref<VertexArray>& vertexArray, U32 vertexCount) = 0;
		virtual void DrawLineIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount = 0) = 0;
		virtual void SetLineThickness(float thickness) = 0;

		virtual void DispatchCompute(U32 x, U32 y, U32 z) = 0;
		virtual void SetImageAccessBarrier() = 0;

		virtual void ToggleBlend(bool bEnable) = 0;
		virtual void ToggleCullFace(bool bEnable) = 0;
		virtual void SetCullFaceMode(bool bIsBack) = 0;
		virtual void ToggleDepthTest(bool bEnable) = 0;
		virtual void ToggleDepthWrite(bool bEnable) = 0;
		virtual void ToggleDepthClamp(bool bEnable) = 0;

		static API GetAPI() { return s_API; }
		static bool Is2D() { return s_bIs2D; }

		static Scope<RendererAPI> Create();

	private:
		static API s_API;
		static bool s_bIs2D;

	};

}
