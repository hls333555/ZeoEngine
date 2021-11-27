#pragma once

#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(void* vertices, uint32_t size);
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
		OpenGLIndexBuffer(uint32_t count);
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

		virtual void* GetColorAttachment(uint32_t index = 0) const override
		{
			ZE_CORE_ASSERT(index < m_ColorAttachments.size());
			return (void*)(intptr_t)m_ColorAttachments[index];
		}

		void Invalidate();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void ReadPixel(uint32_t attachmentIndex, int32_t x, int32_t y, void* outPixelData) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int32_t clearValue) override;
		virtual void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& clearValue) override;

		virtual void Snapshot(const std::string& imagePath, uint32_t captureWidth) override;

	private:
		FrameBufferSpec m_Spec;
		uint32_t m_RendererID = 0;

		std::vector<FrameBufferTextureSpec> m_ColorAttachmentSpecs;
		FrameBufferTextureSpec m_DepthAttachmentSpec;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, uint32_t size = 0, uint32_t offset = 0) override;

		virtual void Bind() const override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size = 0;
		uint32_t m_Binding = 0;
	};

}
