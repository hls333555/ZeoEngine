#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include <glad/glad.h>
#include <stb_image_write.h>

namespace ZeoEngine {

	//////////////////////////////////////////////////////////////////////////
	// VertexBuffer //////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		ZE_PROFILE_FUNCTION();

		// Call glCreateBuffers() instead of glGenBuffers() just for consistency
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		ZE_PROFILE_FUNCTION();

		// Call glCreateBuffers() instead of glGenBuffers() just for consistency
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		ZE_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		ZE_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		ZE_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	//////////////////////////////////////////////////////////////////////////
	// IndexBuffer ///////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		ZE_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		ZE_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		ZE_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		ZE_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameBuffer ///////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static const uint32_t s_MaxFrameBufferSize = 8192;

	namespace Utils {

		static bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		static GLenum TextureTarget(bool bIsMultiSampled)
		{
			return bIsMultiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool bIsMultiSampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(bIsMultiSampled), count, outID);
		}

		static void BindTexture(bool bIsMultiSampled, uint32_t ID)
		{
			glBindTexture(TextureTarget(bIsMultiSampled), ID);
		}

		static void AttachColorTexture(uint32_t ID, uint32_t samples, GLenum format, uint32_t width, uint32_t height, uint32_t index)
		{
			bool bIsMultiSampled = samples > 1;
			if (bIsMultiSampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(bIsMultiSampled), ID, 0);
		}

		static void AttachDepthTexture(uint32_t ID, uint32_t samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool bIsMultiSampled = samples > 1;
			if (bIsMultiSampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(bIsMultiSampled), ID, 0);
		}

	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpec& spec)
		: m_Spec(spec)
	{
		ZE_PROFILE_FUNCTION();

		for (auto spec : m_Spec.Attachments.TextureSpecs)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
			{
				m_ColorAttachmentSpecs.emplace_back(spec);
			}
			else
			{
				m_DepthAttachmentSpec = spec;
			}
		}

		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		ZE_PROFILE_FUNCTION();

		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(static_cast<uint32_t>(m_ColorAttachments.size()), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		ZE_PROFILE_FUNCTION();

		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(static_cast<uint32_t>(m_ColorAttachments.size()), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool bIsMultiSampled = m_Spec.Samples > 1;

		// Attachments
		if (m_ColorAttachmentSpecs.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
			Utils::CreateTextures(bIsMultiSampled, m_ColorAttachments.data(), static_cast<uint32_t>(m_ColorAttachments.size()));

			for (size_t i = 0; i < m_ColorAttachments.size(); ++i)
			{
				Utils::BindTexture(bIsMultiSampled, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecs[i].TextureFormat)
				{
				case FrameBufferTextureFormat::RGBA8:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Spec.Samples, GL_RGBA8, m_Spec.Width, m_Spec.Height, static_cast<uint32_t>(i));
					break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None)
		{
			Utils::CreateTextures(bIsMultiSampled, &m_DepthAttachment, 1);
			Utils::BindTexture(bIsMultiSampled, m_DepthAttachment);
			switch (m_DepthAttachmentSpec.TextureFormat)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Spec.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Spec.Width, m_Spec.Height);
				break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			ZE_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Currently we only support 4 color attachments!");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(static_cast<uint32_t>(m_ColorAttachments.size()), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		ZE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		ZE_PROFILE_FUNCTION();

		// Update viewport to framebuffer texture's resolution
		glViewport(0, 0, m_Spec.Width, m_Spec.Height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		int32_t clearValue = -1;
		// TODO: Fixme!
		//glClearTexImage(m_IDAttachment, 0, GL_RED_INTEGER, GL_INT, &clearValue);
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		ZE_PROFILE_FUNCTION();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize)
		{
			ZE_CORE_WARN("Attempted to resize framebuffer to {0}, {1}!", width, height);
			return;
		}

		m_Spec.Width = width;
		m_Spec.Height = height;
		Invalidate();
	}

	void OpenGLFrameBuffer::Snapshot(const std::string& imageName, uint32_t width, uint32_t height, uint32_t imageWidth)
	{
		constexpr int numOfComponents = 4; // RGBA
		uint32_t snapshotWidth = width, snapshotHeight = height;
		if (imageWidth != 0)
		{
			snapshotWidth = snapshotHeight = imageWidth;
		}

		// Read from the framebuffer into the data array
		const uint32_t dataSize = numOfComponents * snapshotWidth * snapshotHeight;
		GLubyte* data = new GLubyte[dataSize];
		memset(data, 0, dataSize);
		// Resize viewport to center-squared if imageWidth is non-zero
		glViewport((width - snapshotWidth) / 2, (height - snapshotHeight) / 2, snapshotWidth, snapshotHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glReadPixels((width - snapshotWidth) / 2, (height - snapshotHeight) / 2, snapshotWidth, snapshotHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// Write the PNG image
		int strideInBytes = snapshotWidth * numOfComponents;
		stbi_flip_vertically_on_write(1);
		stbi_write_png(imageName.c_str(), snapshotWidth, snapshotHeight, numOfComponents, data, strideInBytes);
		delete[] data;
	}

}
