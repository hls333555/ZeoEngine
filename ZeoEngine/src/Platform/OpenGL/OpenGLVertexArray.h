#pragma once

#include "Engine/Renderer/VertexArray.h"

namespace ZeoEngine {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		virtual void SetIndexBufferData(const void* data, U32 size) const override;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VBOs; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IBO; }

	private:
		U32 m_RendererID;
		U32 m_VertexBufferIndex = 0;
		std::vector<Ref<VertexBuffer>> m_VBOs;
		Ref<IndexBuffer> m_IBO;

	};
}
