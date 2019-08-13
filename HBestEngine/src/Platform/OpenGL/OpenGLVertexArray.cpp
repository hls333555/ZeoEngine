#include "HBEpch.h"
#include "OpenGLVertexArray.h"

#include "glad/glad.h"

namespace HBestEngine {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case HBestEngine::ShaderDataType::Float:
		case HBestEngine::ShaderDataType::Float2:
		case HBestEngine::ShaderDataType::Float3:
		case HBestEngine::ShaderDataType::Float4:
		case HBestEngine::ShaderDataType::Mat3:
		case HBestEngine::ShaderDataType::Mat4:
			return GL_FLOAT;
		case HBestEngine::ShaderDataType::Int:
		case HBestEngine::ShaderDataType::Int2:
		case HBestEngine::ShaderDataType::Int3:
		case HBestEngine::ShaderDataType::Int4:
			return GL_INT;
		case HBestEngine::ShaderDataType::Bool:
			return GL_BOOL;
		default:
			HBE_CORE_ASSERT(false, "Unknown ShaderDataType!");
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

		HBE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

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
