#pragma once

#include "Engine/Renderer/Buffer.h"

namespace ZeoEngine {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(U32 size);
		OpenGLVertexBuffer(void* vertices, U32 size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, U32 size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }	

	private:
		U32 m_RendererID;
		BufferLayout m_Layout;
	
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(U32 count);
		OpenGLIndexBuffer(U32* indices, U32 count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual U32 GetCount() const override { return m_Count; }

	private:
		U32 m_RendererID;
		U32 m_Count;

	};

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		/**
		 * Create an OpenGL frame buffer object.
		 * @param spec - Specification for the FBO
		 * @param textureBindingAttachmentIndex - The index of the attachment whose texture will be bound when called, the order is from color attachments to depth attachment
		 * @param textureBindingSlot - The slot the attachment texture will be bound to when called
		 */
		OpenGLFrameBuffer(const FrameBufferSpec& spec, I32 textureBindingAttachmentIndex = -1, U32 textureBindingSlot = 0);
		virtual ~OpenGLFrameBuffer();

		virtual U32 GetFrameBufferID() const override { return m_RendererID; }
		const FrameBufferSpec& GetSpec() const override { return m_Spec; }

		virtual const Ref<Texture>& GetColorAttachment(U32 index = 0) const override
		{
			ZE_CORE_ASSERT(index < m_ColorAttachments.size());
			return m_ColorAttachments[index];
		}
		virtual const Ref<Texture>& GetDepthAttachment() const override { return m_DepthAttachment; }

		void Invalidate();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindAsBuffer() const override;
		virtual void UnbindAsBuffer() const override;

		virtual void Resize(U32 width, U32 height) override;

		virtual void ReadPixel(U32 attachmentIndex, I32 x, I32 y, void* outPixelData) override;

		virtual void ClearColorAttachment(U32 attachmentIndex, I32 clearValue) override;
		virtual void ClearColorAttachment(U32 attachmentIndex, const Vec4& clearValue) override;

		virtual void BlitColorTo(const Ref<FrameBuffer>& targetFBO, U32 attachmentIndex, U32 targetAttachmentIndex) override;
		virtual void BlitDepthTo(const Ref<FrameBuffer>& targetFBO, bool bIncludeStencil = true) override;

		virtual void Snapshot(const std::string& imagePath, U32 captureWidth) override;

	private:
		void Cleanup();

	private:
		FrameBufferSpec m_Spec;
		U32 m_RendererID = 0;

		std::vector<FrameBufferTextureSpec> m_ColorAttachmentSpecs;
		FrameBufferTextureSpec m_DepthAttachmentSpec;

		std::vector<Ref<Texture>> m_ColorAttachments;
		Ref<Texture> m_DepthAttachment;

		I32 m_TextureBindingAttachmentIndex;
		U32 m_TextureBindingSlot;

	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(U32 size, U32 binding);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, U32 size = 0, U32 offset = 0) override;

		virtual void Bind() const override;

	private:
		U32 m_RendererID = 0;
		U32 m_Size = 0;
		U32 m_Binding = 0;
	};

}
