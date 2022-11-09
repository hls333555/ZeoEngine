#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		// NOTE: Add a breakpoint below to see where the error was generated
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         ZE_CORE_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       ZE_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          ZE_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: ZE_CORE_TRACE(message); return;
		}

		ZE_CORE_ASSERT(false, "Unknown severity level!");
	}

	void OpenGLRendererAPI::Init()
	{
#ifdef ZE_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (Is2D())
		{
			glEnable(GL_LINE_SMOOTH);
		}
	}

	void OpenGLRendererAPI::SetViewport(U32 x, U32 y, U32 width, U32 height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const Vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear(ClearType type)
	{
		switch (type)
		{
			case ClearType::Color_Depth_Stencil:
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				break;
			case ClearType::Depth:
				glClear(GL_DEPTH_BUFFER_BIT);
				break;
		}
	}

	void OpenGLRendererAPI::DrawArrays(U32 vertexCount)
	{
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount, I32 baseIndex)
	{
		vertexArray->Bind();
		U32 count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		void* offset = reinterpret_cast<void*>(sizeof(U32) * baseIndex);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, offset);
	}

	void OpenGLRendererAPI::DrawIndexed(I32 baseVertex, U32 indexCount, I32 baseIndex)
	{
		void* indices = reinterpret_cast<void*>(sizeof(U32) * baseIndex);
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indices, baseVertex);
	}

	void OpenGLRendererAPI::DrawInstanced(U32 instanceCount)
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);
	}

	void OpenGLRendererAPI::DrawLine(const Ref<VertexArray>& vertexArray, U32 vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawLineIndexed(const Ref<VertexArray>& vertexArray, U32 indexCount)
	{
		vertexArray->Bind();
		U32 count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, 0);
	}

	void OpenGLRendererAPI::SetLineThickness(float thickness)
	{
		glLineWidth(thickness);
	}

	void OpenGLRendererAPI::DispatchCompute(U32 x, U32 y, U32 z)
	{
		glDispatchCompute(x, y, z);
	}

	void OpenGLRendererAPI::SetImageAccessBarrier()
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void OpenGLRendererAPI::ToggleBlend(bool bEnable)
	{
		if (bEnable)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void OpenGLRendererAPI::ToggleCullFace(bool bEnable)
	{
		if (bEnable)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
	}

	void OpenGLRendererAPI::SetCullFaceMode(bool bIsBack)
	{
		glCullFace(bIsBack ? GL_BACK : GL_FRONT);
	}

	void OpenGLRendererAPI::ToggleDepthTest(bool bEnable)
	{
		if (bEnable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}

	void OpenGLRendererAPI::ToggleDepthWrite(bool bEnable)
	{
		glDepthMask(bEnable ? GL_TRUE : GL_FALSE);
	}

	void OpenGLRendererAPI::ToggleDepthClamp(bool bEnable)
	{
		if (bEnable)
		{
			glEnable(GL_DEPTH_CLAMP);
		}
		else
		{
			glDisable(GL_DEPTH_CLAMP);
		}
	}

}
