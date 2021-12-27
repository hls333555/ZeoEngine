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
		ZE_PROFILE_FUNCTION();

#ifdef ZE_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (RendererAPI::Is2D())
		{
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_LINE_SMOOTH);
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
		}

	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
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

	void OpenGLRendererAPI::DrawArrays(uint32_t vertexCount)
	{
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, int32_t baseIndex)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		void* offset = reinterpret_cast<void*>(sizeof(uint32_t) * baseIndex);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, offset);
	}

	void OpenGLRendererAPI::DrawIndexed(int32_t baseVertex, uint32_t indexCount, int32_t baseIndex)
	{
		void* indices = reinterpret_cast<void*>(sizeof(uint32_t) * baseIndex);
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indices, baseVertex);
	}

	void OpenGLRendererAPI::DrawInstanced(uint32_t instanceCount)
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);
	}

	void OpenGLRendererAPI::DrawLine(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawLineIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, 0);
	}

	void OpenGLRendererAPI::SetLineThickness(float thickness)
	{
		glLineWidth(thickness);
	}

	void OpenGLRendererAPI::ToggleFaceCulling(bool bEnable)
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

	void OpenGLRendererAPI::SetFaceCullingMode(bool bIsBack)
	{
		glCullFace(bIsBack ? GL_BACK : GL_FRONT);
	}

	void OpenGLRendererAPI::ToggleDepthTesting(bool bEnable)
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

	void OpenGLRendererAPI::ToggleDepthWriting(bool bEnable)
	{
		glDepthMask(bEnable ? GL_TRUE : GL_FALSE);
	}

	void OpenGLRendererAPI::ToggleDepthClamping(bool bEnable)
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
