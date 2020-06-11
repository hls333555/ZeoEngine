#pragma once

#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }	

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;

	};

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpec& spec);
		virtual ~OpenGLFrameBuffer();

		const FrameBufferSpec& GetSpec() const override { return m_Spec; }

		void Invalidate();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void* GetColorAttachment() const override { return (void*)(intptr_t)m_ColorAttachment; }

	private:
		FrameBufferSpec m_Spec;
		uint32_t m_RendererID, m_ColorAttachment, m_DepthAttachment;

	};

}
