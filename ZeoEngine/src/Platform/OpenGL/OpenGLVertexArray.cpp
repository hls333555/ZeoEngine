#include "ZEpch.h"
#include "OpenGLVertexArray.h"

#include "glad/glad.h"

namespace ZeoEngine {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ZeoEngine::ShaderDataType::Float:
		case ZeoEngine::ShaderDataType::Float2:
		case ZeoEngine::ShaderDataType::Float3:
		case ZeoEngine::ShaderDataType::Float4:
		case ZeoEngine::ShaderDataType::Mat3:
		case ZeoEngine::ShaderDataType::Mat4:
			return GL_FLOAT;
		case ZeoEngine::ShaderDataType::Int:
		case ZeoEngine::ShaderDataType::Int2:
		case ZeoEngine::ShaderDataType::Int3:
		case ZeoEngine::ShaderDataType::Int4:
			return GL_INT;
		case ZeoEngine::ShaderDataType::Bool:
			return GL_BOOL;
		default:
			ZE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		ZE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		const auto& layout = vertexBuffer->GetLayout();
		const auto& elements = layout.GetElements();
		for (uint32_t i = 0; i < elements.size(); ++i)
		{
			const auto& element = elements[i];
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.bNormalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
		}

		m_VBOs.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IBO = indexBuffer;
	}

}
