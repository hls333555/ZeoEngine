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

	OpenGLVertexBuffer::OpenGLVertexBuffer(U32 size)
	{
		ZE_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, U32 size)
	{
		ZE_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, vertices, GL_STATIC_DRAW);
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

	void OpenGLVertexBuffer::SetData(const void* data, U32 size)
	{
		//glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	//////////////////////////////////////////////////////////////////////////
	// IndexBuffer ///////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(U32 count)
	{
		ZE_PROFILE_FUNCTION();

		// Call glCreateBuffers() instead of glGenBuffers() just for consistency
		glCreateBuffers(1, &m_RendererID);
		// Since index buffer state is managed by vao, index buffer data is manipulated in VertexArray::SetIndexBufferData
		glNamedBufferData(m_RendererID, count * sizeof(U32), nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(U32* indices, U32 count)
		: m_Count(count)
	{
		ZE_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, count * sizeof(U32), indices, GL_STATIC_DRAW);
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

	static const U32 s_MaxFrameBufferSize = 8192;

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

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpec& spec, I32 textureBindingAttachmentIndex, U32 textureBindingSlot)
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

	static constexpr GLenum s_DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

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
		const U32 colorCount = static_cast<U32>(m_ColorAttachmentSpecs.size());
		if (colorCount)
		{
			m_ColorAttachments.reserve(colorCount);
			// TODO: Add texture array support?
			for (U32 i = 0; i < colorCount; ++i)
			{
				auto texture = Texture2D::Create("ZID_ColorAttachment" + std::to_string(i), m_Spec.Width, m_Spec.Height, m_ColorAttachmentSpecs[i].TextureFormat);
				U32 id = (U32)(intptr_t)texture->GetTextureID();
				glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0 + i, id, 0);
				m_ColorAttachments.emplace_back(std::move(texture));
			}
		}

		// Depth attachment
		auto format = m_DepthAttachmentSpec.TextureFormat;
		if (format != TextureFormat::None)
		{
			const U32 arraySize = m_DepthAttachmentSpec.TextureArraySize;
			GLenum attachmentType = Utils::ToGLDepthAttachment(format);
			if (arraySize > 1)
			{
				m_DepthAttachment = Texture2DArray::Create(m_Spec.Width, m_Spec.Height, arraySize, format);
			}
			else
			{
				m_DepthAttachment = Texture2D::Create("ZID_DepthAttachment", m_Spec.Width, m_Spec.Height, format);
			}
			U32 id = (U32)(intptr_t)m_DepthAttachment->GetTextureID();
			glNamedFramebufferTexture(m_RendererID, attachmentType, id, 0);
		}

		if (m_ColorAttachments.size() > 1)
		{
			ZE_CORE_ASSERT(m_ColorAttachments.size() <= sizeof(s_DrawBuffers) / sizeof(s_DrawBuffers[0]), "Currently we only support up to 4 color attachments!");
			glNamedFramebufferDrawBuffers(m_RendererID, static_cast<GLsizei>(m_ColorAttachments.size()), s_DrawBuffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glNamedFramebufferDrawBuffer(m_RendererID, GL_NONE);
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
			for (SizeT i = 0; i < samplers.size(); ++i)
			{
				const auto& colorAttachment = m_ColorAttachments[m_TextureBindingAttachmentIndex];
				colorAttachment->ChangeSampler(samplers[i]);
				colorAttachment->SetBindingSlot(m_TextureBindingSlot + static_cast<U32>(i));
				colorAttachment->Bind();
			}
		}
		else if (m_TextureBindingAttachmentIndex == colorAttachmentCount)
		{
			const auto& samplers = m_DepthAttachmentSpec.TextureSamplers;
			for (SizeT i = 0; i < samplers.size(); ++i)
			{
				m_DepthAttachment->ChangeSampler(samplers[i]);
				m_DepthAttachment->SetBindingSlot(m_TextureBindingSlot + static_cast<U32>(i));
				m_DepthAttachment->Bind();
			}
		}
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		if (m_TextureBindingAttachmentIndex < 0) return;

		const auto colorAttachmentCount = m_ColorAttachments.size();
		if (m_TextureBindingAttachmentIndex < colorAttachmentCount)
		{
			const auto& samplers = m_ColorAttachmentSpecs[m_TextureBindingAttachmentIndex].TextureSamplers;
			for (SizeT i = 0; i < samplers.size(); ++i)
			{
				const auto& colorAttachment = m_ColorAttachments[m_TextureBindingAttachmentIndex];
				colorAttachment->ChangeSampler(samplers[i]);
				colorAttachment->SetBindingSlot(m_TextureBindingSlot + static_cast<U32>(i));
				colorAttachment->Unbind();
			}
		}
		else if (m_TextureBindingAttachmentIndex == colorAttachmentCount)
		{
			const auto& samplers = m_DepthAttachmentSpec.TextureSamplers;
			for (SizeT i = 0; i < samplers.size(); ++i)
			{
				m_DepthAttachment->ChangeSampler(samplers[i]);
				m_DepthAttachment->SetBindingSlot(m_TextureBindingSlot + static_cast<U32>(i));
				m_DepthAttachment->Unbind();
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

	void OpenGLFrameBuffer::Resize(U32 width, U32 height)
	{
		if (m_Spec.bFixedSize) return;

		if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize)
		{
			ZE_CORE_WARN("Attempted to resize framebuffer to {0}, {1}!", width, height);
			return;
		}

		m_Spec.Width = width;
		m_Spec.Height = height;
		Invalidate();
	}

	void OpenGLFrameBuffer::ReadPixel(U32 attachmentIndex, I32 x, I32 y, void* outPixelData)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		auto format = m_ColorAttachmentSpecs[attachmentIndex].TextureFormat;
		glReadPixels(x, y, 1, 1, OpenGLUtils::ToGLTextureFormat(format), OpenGLUtils::ToGLDataType(format), outPixelData);
	}

	void OpenGLFrameBuffer::ClearColorAttachment(U32 attachmentIndex, I32 clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		U32 id = (U32)(intptr_t)m_ColorAttachments[attachmentIndex]->GetTextureID();
		glClearTexImage(id, 0, OpenGLUtils::ToGLTextureFormat(spec.TextureFormat), OpenGLUtils::ToGLDataType(spec.TextureFormat), &clearValue);
	}

	void OpenGLFrameBuffer::ClearColorAttachment(U32 attachmentIndex, const Vec4& clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		U32 id = (U32)(intptr_t)m_ColorAttachments[attachmentIndex]->GetTextureID();
		glClearTexImage(id, 0, OpenGLUtils::ToGLTextureFormat(spec.TextureFormat), OpenGLUtils::ToGLDataType(spec.TextureFormat), glm::value_ptr(clearValue));
	}

	void OpenGLFrameBuffer::BlitColorTo(const Ref<FrameBuffer>& targetFBO, U32 attachmentIndex, U32 targetAttachmentIndex)
	{
		const auto targetRendererID = targetFBO->GetFrameBufferID();
		glNamedFramebufferReadBuffer(m_RendererID, GL_COLOR_ATTACHMENT0 + attachmentIndex);
		glNamedFramebufferDrawBuffer(targetRendererID, GL_COLOR_ATTACHMENT0 + targetAttachmentIndex);
		glBlitNamedFramebuffer(m_RendererID, targetRendererID,
			0, 0, m_Spec.Width, m_Spec.Height,
			0, 0, targetFBO->GetSpec().Width, targetFBO->GetSpec().Height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		// Reset
		glNamedFramebufferDrawBuffers(targetRendererID, static_cast<GLsizei>(m_ColorAttachments.size()), s_DrawBuffers);
	}

	void OpenGLFrameBuffer::BlitDepthTo(const Ref<FrameBuffer>& targetFBO, bool bIncludeStencil)
	{
		const auto targetRendererID = targetFBO->GetFrameBufferID();
		glBlitNamedFramebuffer(m_RendererID, targetRendererID,
			0, 0, m_Spec.Width, m_Spec.Height,
			0, 0, targetFBO->GetSpec().Width, targetFBO->GetSpec().Height,
			bIncludeStencil ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}

	void OpenGLFrameBuffer::Snapshot(const std::string& imagePath, U32 captureWidth)
	{
		if (captureWidth <= 0)
		{
			ZE_CORE_ERROR("Capture width cannot be <= 0!");
			return;
		}

		U32 width = glm::min(m_Spec.Width, m_Spec.Height);
		width = glm::min(width, captureWidth);

		constexpr int numOfComponents = 4; // RGBA

		// Read from the framebuffer into the data array
		const U32 dataSize = numOfComponents * width * width;
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

	OpenGLUniformBuffer::OpenGLUniformBuffer(U32 size, U32 binding)
		: m_Size(size), m_Binding(binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, U32 size, U32 offset)
	{
		//glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_RendererID);
		glNamedBufferSubData(m_RendererID, offset, size == 0 ? m_Size : size, data);
	}

	void OpenGLUniformBuffer::Bind() const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_RendererID);
	}

}
