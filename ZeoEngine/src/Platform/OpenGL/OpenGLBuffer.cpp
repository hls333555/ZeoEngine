#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include <glad/glad.h>
#include <stb_image_write.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, uint32_t size)
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

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count)
	{
		ZE_PROFILE_FUNCTION();

		// Call glCreateBuffers() instead of glGenBuffers() just for consistency
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		// Since index buffer state is managed by vao, index buffer data is manipulated in VertexArray::SetIndexBufferData
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	}

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

		static void AttachColorTexture(uint32_t ID, uint32_t samples, GLenum internalFormat, GLenum format, GLenum dataType, uint32_t width, uint32_t height, uint32_t index)
		{
			bool bIsMultiSampled = samples > 1;
			if (bIsMultiSampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);

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

		static GLenum ToGLTextureInternalFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
				case FrameBufferTextureFormat::RGBA8:		return GL_RGBA8;
				case FrameBufferTextureFormat::RGBA16F:		return GL_RGBA16F;
				case FrameBufferTextureFormat::RED_INTEGER:	return GL_R32I;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}

		static GLenum ToGLTextureFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
				case FrameBufferTextureFormat::RGBA8:
				case FrameBufferTextureFormat::RGBA16F:			return GL_RGBA;
				case FrameBufferTextureFormat::RED_INTEGER:		return GL_RED_INTEGER;
				case FrameBufferTextureFormat::DEPTH24STENCIL8:	return GL_DEPTH24_STENCIL8;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}

		static GLenum ToGLDataType(FrameBufferTextureFormat format)
		{
			switch (format)
			{
				case FrameBufferTextureFormat::RGBA8:		return GL_UNSIGNED_BYTE;
				case FrameBufferTextureFormat::RGBA16F:		return GL_FLOAT;
				case FrameBufferTextureFormat::RED_INTEGER:	return GL_INT;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
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
				auto format = m_ColorAttachmentSpecs[i].TextureFormat;
				Utils::AttachColorTexture(m_ColorAttachments[i], m_Spec.Samples,
					Utils::ToGLTextureInternalFormat(format),
					Utils::ToGLTextureFormat(format),
					Utils::ToGLDataType(format),
					m_Spec.Width, m_Spec.Height, static_cast<uint32_t>(i));
			}
		}

		auto format = m_DepthAttachmentSpec.TextureFormat;
		if (format != FrameBufferTextureFormat::None)
		{
			Utils::CreateTextures(bIsMultiSampled, &m_DepthAttachment, 1);
			Utils::BindTexture(bIsMultiSampled, m_DepthAttachment);
			Utils::AttachDepthTexture(m_DepthAttachment, m_Spec.Samples, Utils::ToGLTextureFormat(format), GL_DEPTH_STENCIL_ATTACHMENT, m_Spec.Width, m_Spec.Height);
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

	void OpenGLFrameBuffer::ReadPixel(uint32_t attachmentIndex, int32_t x, int32_t y, void* outPixelData)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		auto format = m_ColorAttachmentSpecs[attachmentIndex].TextureFormat;
		glReadPixels(x, y, 1, 1, Utils::ToGLTextureFormat(format), Utils::ToGLDataType(format), outPixelData);
	}

	void OpenGLFrameBuffer::ClearAttachment(uint32_t attachmentIndex, int32_t clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::ToGLTextureFormat(spec.TextureFormat), Utils::ToGLDataType(spec.TextureFormat), &clearValue);
	}

	void OpenGLFrameBuffer::ClearAttachment(uint32_t attachmentIndex, const glm::vec4& clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::ToGLTextureFormat(spec.TextureFormat), Utils::ToGLDataType(spec.TextureFormat), glm::value_ptr(clearValue));
	}

	void OpenGLFrameBuffer::Snapshot(const std::string& imagePath, uint32_t captureWidth)
	{
		if (captureWidth <= 0)
		{
			ZE_CORE_ERROR("Capture width cannot be <= 0!");
			return;
		}

		uint32_t width = glm::min(m_Spec.Width, m_Spec.Height);
		width = glm::min(width, captureWidth);

		constexpr int numOfComponents = 4; // RGBA

		// Read from the framebuffer into the data array
		const uint32_t dataSize = numOfComponents * width * width;
		GLubyte* data = new GLubyte[dataSize];
		memset(data, 0, dataSize);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		auto format = m_ColorAttachmentSpecs[0].TextureFormat;
		glReadPixels((m_Spec.Width - width) / 2, (m_Spec.Height - width) / 2, width, width, Utils::ToGLTextureFormat(format), Utils::ToGLDataType(format), data);

		// Write the PNG image
		int strideInBytes = width * numOfComponents;
		stbi_flip_vertically_on_write(1);
		stbi_write_png(imagePath.c_str(), width, width, numOfComponents, data, strideInBytes);
		delete[] data;
	}

	//////////////////////////////////////////////////////////////////////////
	// UniformBuffer /////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_Binding(binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_RendererID);
		glNamedBufferSubData(m_RendererID, offset, size == 0 ? m_Size : size, data);
	}

	void OpenGLUniformBuffer::Bind() const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_RendererID);
	}

}
