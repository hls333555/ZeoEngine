#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include <glad/glad.h>
#include <stb_image_write.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLUtils.h"

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

		static GLenum ToGLDepthAttachment(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::DEPTH32F:			return GL_DEPTH_ATTACHMENT;
				case TextureFormat::DEPTH24STENCIL8:	return GL_DEPTH_STENCIL_ATTACHMENT;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}

		static bool IsDepthFormat(TextureFormat format)
		{
			return format > TextureFormat::_DEPTH_START_;
		}

	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpec& spec, int32_t textureBindingAttachmentIndex, uint32_t textureBindingSlot)
		: m_Spec(spec), m_TextureBindingAttachmentIndex(textureBindingAttachmentIndex), m_TextureBindingSlot(textureBindingSlot)
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

		Cleanup();
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		ZE_PROFILE_FUNCTION();

		if (m_RendererID)
		{
			Cleanup();
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool bIsMultiSampled = m_Spec.Samples > 1;

		// Color attachments
		const uint32_t colorCount = static_cast<uint32_t>(m_ColorAttachmentSpecs.size());
		if (colorCount)
		{
			m_ColorAttachments.reserve(colorCount);
			// TODO: Add texture array support?
			for (uint32_t i = 0; i < colorCount; ++i)
			{
				auto texture = Texture2D::Create("ZID_ColorAttachment" + std::to_string(i), m_Spec.Width, m_Spec.Height, m_ColorAttachmentSpecs[i].TextureFormat);
				uint32_t id = (uint32_t)(intptr_t)texture->GetTextureID();
				glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0 + i, id, 0);
				m_ColorAttachments.emplace_back(std::move(texture));
			}
		}

		// Depth attachment
		auto format = m_DepthAttachmentSpec.TextureFormat;
		if (format != TextureFormat::None)
		{
			const uint32_t arraySize = m_DepthAttachmentSpec.TextureArraySize;
			GLenum attachmentType = Utils::ToGLDepthAttachment(format);
			if (arraySize > 1)
			{
				m_DepthAttachment = Texture2DArray::Create(m_Spec.Width, m_Spec.Height, arraySize, format);
			}
			else
			{
				m_DepthAttachment = Texture2D::Create("ZID_DepthAttachment", m_Spec.Width, m_Spec.Height, format);
			}
			uint32_t id = (uint32_t)(intptr_t)m_DepthAttachment->GetTextureID();
			glNamedFramebufferTexture(m_RendererID, attachmentType, id, 0);
		}

		if (m_ColorAttachments.size() > 1)
		{
			ZE_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Currently we only support up to 4 color attachments!");
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
		if (m_TextureBindingAttachmentIndex < 0) return;

		auto colorAttachmentCount = m_ColorAttachments.size();
		if (m_TextureBindingAttachmentIndex < colorAttachmentCount)
		{
			const auto& samplers = m_ColorAttachmentSpecs[m_TextureBindingAttachmentIndex].TextureSamplers;
			for (size_t i = 0; i < samplers.size(); ++i)
			{
				m_ColorAttachments[m_TextureBindingAttachmentIndex]->ChangeSampler(samplers[i]);
				m_ColorAttachments[m_TextureBindingAttachmentIndex]->Bind(m_TextureBindingSlot + static_cast<uint32_t>(i));
			}
		}
		else if (m_TextureBindingAttachmentIndex == colorAttachmentCount)
		{
			const auto& samplers = m_DepthAttachmentSpec.TextureSamplers;
			for (size_t i = 0; i < samplers.size(); ++i)
			{
				m_DepthAttachment->ChangeSampler(samplers[i]);
				m_DepthAttachment->Bind(m_TextureBindingSlot + static_cast<uint32_t>(i));
			}
		}
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		if (m_TextureBindingAttachmentIndex < 0) return;

		auto colorAttachmentCount = m_ColorAttachments.size();
		if (m_TextureBindingAttachmentIndex < colorAttachmentCount)
		{
			const auto& samplers = m_ColorAttachmentSpecs[m_TextureBindingAttachmentIndex].TextureSamplers;
			for (size_t i = 0; i < samplers.size(); ++i)
			{
				m_ColorAttachments[m_TextureBindingAttachmentIndex]->ChangeSampler(samplers[i]);
				m_ColorAttachments[m_TextureBindingAttachmentIndex]->Unbind(m_TextureBindingSlot + static_cast<uint32_t>(i));
			}
		}
		else if (m_TextureBindingAttachmentIndex == colorAttachmentCount)
		{
			const auto& samplers = m_DepthAttachmentSpec.TextureSamplers;
			for (size_t i = 0; i < samplers.size(); ++i)
			{
				m_DepthAttachment->ChangeSampler(samplers[i]);
				m_DepthAttachment->Unbind(m_TextureBindingSlot + static_cast<uint32_t>(i));
			}
		}
	}

	void OpenGLFrameBuffer::BindAsBuffer() const
	{
		ZE_PROFILE_FUNCTION();

		// Update viewport to framebuffer texture's resolution
		glViewport(0, 0, m_Spec.Width, m_Spec.Height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFrameBuffer::UnbindAsBuffer() const
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
		glReadPixels(x, y, 1, 1, OpenGLUtils::ToGLTextureFormat(format), OpenGLUtils::ToGLDataType(format), outPixelData);
	}

	void OpenGLFrameBuffer::ClearColorAttachment(uint32_t attachmentIndex, int32_t clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		uint32_t id = (uint32_t)(intptr_t)m_ColorAttachments[attachmentIndex]->GetTextureID();
		glClearTexImage(id, 0, OpenGLUtils::ToGLTextureFormat(spec.TextureFormat), OpenGLUtils::ToGLDataType(spec.TextureFormat), &clearValue);
	}

	void OpenGLFrameBuffer::ClearColorAttachment(uint32_t attachmentIndex, const glm::vec4& clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		uint32_t id = (uint32_t)(intptr_t)m_ColorAttachments[attachmentIndex]->GetTextureID();
		glClearTexImage(id, 0, OpenGLUtils::ToGLTextureFormat(spec.TextureFormat), OpenGLUtils::ToGLDataType(spec.TextureFormat), glm::value_ptr(clearValue));
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
		glReadPixels((m_Spec.Width - width) / 2, (m_Spec.Height - width) / 2, width, width, OpenGLUtils::ToGLTextureFormat(format), OpenGLUtils::ToGLDataType(format), data);

		// Write the PNG image
		int strideInBytes = width * numOfComponents;
		stbi_flip_vertically_on_write(1);
		stbi_write_png(imagePath.c_str(), width, width, numOfComponents, data, strideInBytes);
		delete[] data;
	}

	void OpenGLFrameBuffer::Cleanup()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		m_ColorAttachments.clear();
		m_DepthAttachment = nullptr;
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
