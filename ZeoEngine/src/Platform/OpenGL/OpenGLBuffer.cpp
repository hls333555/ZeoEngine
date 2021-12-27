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
				case FrameBufferTextureFormat::DEPTH32F:		return GL_DEPTH_COMPONENT32F;
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

		static GLenum ToGLDepthAttachment(FrameBufferTextureFormat format)
		{
			switch (format)
			{
				case FrameBufferTextureFormat::DEPTH32F:		return GL_DEPTH_ATTACHMENT;
				case FrameBufferTextureFormat::DEPTH24STENCIL8:	return GL_DEPTH_STENCIL_ATTACHMENT;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}

		static bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			return format > FrameBufferTextureFormat::_DEPTH_START_;
		}

		static GLenum TextureTarget(bool bIsMultiSampled)
		{
			return bIsMultiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static GLenum TextureTargetArray(bool bIsMultiSampled)
		{
			return bIsMultiSampled ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
		}

		static void CreateTextures(bool bIsArray, bool bIsMultiSampled, uint32_t* outIDs, uint32_t count)
		{
			glCreateTextures(bIsArray ? TextureTargetArray(bIsMultiSampled) : TextureTarget(bIsMultiSampled), count, outIDs);
		}

		static void CreateSamplers(uint32_t count, uint32_t* outIDs)
		{
			glCreateSamplers(count, outIDs);
		}

		static void CreateDepthViews(int32_t arraySize, uint32_t ID, uint32_t* viewIDs, GLenum format)
		{
			glGenTextures(arraySize, viewIDs);
			for (int32_t i = 0; i < arraySize; ++i)
			{
				glTextureView(viewIDs[i], GL_TEXTURE_2D, ID, format, 0, 1, i, 1);
			}
		}

		static void BindTexture(bool bIsArray, bool bIsMultiSampled, uint32_t ID)
		{
			glBindTexture(bIsArray ? TextureTargetArray(bIsMultiSampled) : TextureTarget(bIsMultiSampled), ID);
		}

		static void BindTextureToSlot(uint32_t slot, uint32_t ID)
		{
			glBindTextureUnit(slot, ID);
		}

		static void BindSampler(uint32_t slot, uint32_t ID)
		{
			glBindSampler(slot, ID);
		}

		static void AttachSamplers(uint32_t* samplers, FrameBufferSamplerType samplerType, uint32_t index)
		{
			GLint filter, wrap;
			switch (samplerType)
			{
				case FrameBufferSamplerType::PointClamp:
				case FrameBufferSamplerType::ShadowDepth:
					filter = GL_NEAREST;
					wrap = GL_CLAMP_TO_EDGE;
					break;
				case FrameBufferSamplerType::PointRepeat:
					filter = GL_NEAREST;
					wrap = GL_REPEAT;
					break;
				case FrameBufferSamplerType::BilinearClamp:
					filter = GL_LINEAR;
					wrap = GL_CLAMP_TO_EDGE;
					break;
				case FrameBufferSamplerType::ShadowPCF:
					filter = GL_LINEAR;
					wrap = GL_CLAMP_TO_EDGE;
					glSamplerParameteri(samplers[index], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
					glSamplerParameteri(samplers[index], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
					break;
			}
			glSamplerParameteri(samplers[index], GL_TEXTURE_MIN_FILTER, filter);
			glSamplerParameteri(samplers[index], GL_TEXTURE_MAG_FILTER, filter);
			glSamplerParameteri(samplers[index], GL_TEXTURE_WRAP_R, wrap);
			glSamplerParameteri(samplers[index], GL_TEXTURE_WRAP_S, wrap);
			glSamplerParameteri(samplers[index], GL_TEXTURE_WRAP_T, wrap);
		}

		static void AttachColorTexture(uint32_t ID, uint32_t samples, FrameBufferTextureFormat textureFormat, uint32_t width, uint32_t height, uint32_t index)
		{
			const bool bIsMultiSampled = samples > 1;
			GLenum internalFormat = Utils::ToGLTextureInternalFormat(textureFormat);
			GLenum format = Utils::ToGLTextureFormat(textureFormat);
			GLenum dataType = Utils::ToGLDataType(textureFormat);
			if (bIsMultiSampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);

				// We cannot use custom sampler due to glBindSampler(0, 0) call in ImGui_ImplOpenGL3_SetupRenderState()
				// So we have to create default sampling methods dedicated for ImGui texture display
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(bIsMultiSampled), ID, 0);
		}

		static void AttachDepthTexture(uint32_t arraySize, uint32_t ID, uint32_t samples, FrameBufferTextureFormat textureFormat, uint32_t width, uint32_t height)
		{
			const bool bIsMultiSampled = samples > 1;
			const bool bIsArray = arraySize > 1;
			GLenum format = Utils::ToGLTextureFormat(textureFormat);
			GLenum attachmentType = Utils::ToGLDepthAttachment(textureFormat);
			if (bIsMultiSampled)
			{
				if (bIsArray)
				{
					glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, samples, format, width, height, arraySize, GL_FALSE);
				}
				else
				{
					glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
				}
			}
			else
			{
				if (bIsArray)
				{
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, format, width, height, arraySize);
				}
				else
				{
					glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
				}

				glTexParameteri(bIsArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(bIsArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(bIsArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(bIsArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(bIsArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			if (bIsArray)
			{
				glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, ID, 0);
			}
			else
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(bIsMultiSampled), ID, 0);
			}
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

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool bIsMultiSampled = m_Spec.Samples > 1;

		// Color attachments
		const uint32_t colorCount = static_cast<uint32_t>(m_ColorAttachmentSpecs.size());
		if (colorCount)
		{
			m_ColorAttachments.resize(colorCount);
			// TODO: Add array support?
			Utils::CreateTextures(false, bIsMultiSampled, m_ColorAttachments.data(), colorCount);

			// Assume each color texture uses only one sampler
			m_ColorSamplers.resize(colorCount);
			Utils::CreateSamplers(colorCount, m_ColorSamplers.data());

			for (uint32_t i = 0; i < colorCount; ++i)
			{
				Utils::BindTexture(false, bIsMultiSampled, m_ColorAttachments[i]);

				auto format = m_ColorAttachmentSpecs[i].TextureFormat;
				Utils::AttachColorTexture(m_ColorAttachments[i], m_Spec.Samples, format, m_Spec.Width, m_Spec.Height, i);

				const auto& samplers = m_ColorAttachmentSpecs[i].TextureSamplerSpecs.Samplers;
				ZE_CORE_ASSERT(samplers.size());
				Utils::AttachSamplers(m_ColorSamplers.data(), samplers[0], i);
			}
		}

		// Depth attachment
		auto format = m_DepthAttachmentSpec.TextureFormat;
		if (format != FrameBufferTextureFormat::None)
		{
			const uint32_t arraySize = m_DepthAttachmentSpec.TextureArraySize;
			const bool bIsArray = arraySize > 1;

			Utils::CreateTextures(bIsArray, bIsMultiSampled, &m_DepthAttachment, 1);
			Utils::BindTexture(bIsArray, bIsMultiSampled, m_DepthAttachment);

			Utils::AttachDepthTexture(arraySize, m_DepthAttachment, m_Spec.Samples, format,	m_Spec.Width, m_Spec.Height);

			const uint32_t samplerCount = static_cast<uint32_t>(m_DepthAttachmentSpec.TextureSamplerSpecs.Samplers.size());
			m_DepthSamplers.resize(samplerCount);
			Utils::CreateSamplers(samplerCount, m_DepthSamplers.data());

			for (uint32_t i = 0; i < samplerCount; ++i)
			{
				Utils::AttachSamplers(m_DepthSamplers.data(), m_DepthAttachmentSpec.TextureSamplerSpecs.Samplers[i], i);
			}

			m_DepthAttachmentViews.resize(arraySize);
			Utils::CreateDepthViews(arraySize, m_DepthAttachment, m_DepthAttachmentViews.data(), Utils::ToGLTextureFormat(format));
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
		bool bIsMultiSampled = m_Spec.Samples > 1;
		if (m_TextureBindingAttachmentIndex < colorAttachmentCount)
		{
			Utils::BindTextureToSlot(m_TextureBindingSlot, m_ColorAttachments[m_TextureBindingAttachmentIndex]);
			Utils::BindSampler(m_TextureBindingSlot, m_ColorSamplers[m_TextureBindingAttachmentIndex]);
		}
		else if (m_TextureBindingAttachmentIndex == colorAttachmentCount)
		{
			const uint32_t samplerCount = static_cast<uint32_t>(m_DepthAttachmentSpec.TextureSamplerSpecs.Samplers.size());
			const uint32_t arraySize = m_DepthAttachmentSpec.TextureArraySize;
			const bool bIsArray = arraySize > 1;
			for (uint32_t i = 0; i < samplerCount; ++i)
			{
				uint32_t slot = m_TextureBindingSlot + i;
				Utils::BindTextureToSlot(slot, m_DepthAttachment);
				Utils::BindSampler(slot, m_DepthSamplers[i]);
			}
		}
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		if (m_TextureBindingAttachmentIndex < 0) return;

		auto colorAttachmentCount = m_ColorAttachments.size();
		if (m_TextureBindingAttachmentIndex == colorAttachmentCount)
		{
			// NOTE: We must unbind shader after draw call which uses these samplers, otherwise errors will keep emitting even if no draw call is issued
			const uint32_t samplerCount = static_cast<uint32_t>(m_DepthAttachmentSpec.TextureSamplerSpecs.Samplers.size());
			for (uint32_t i = 0; i < samplerCount; ++i)
			{
				Utils::BindSampler(m_TextureBindingSlot + i, 0);
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
		glReadPixels(x, y, 1, 1, Utils::ToGLTextureFormat(format), Utils::ToGLDataType(format), outPixelData);
	}

	void OpenGLFrameBuffer::ClearColorAttachment(uint32_t attachmentIndex, int32_t clearValue)
	{
		ZE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::ToGLTextureFormat(spec.TextureFormat), Utils::ToGLDataType(spec.TextureFormat), &clearValue);
	}

	void OpenGLFrameBuffer::ClearColorAttachment(uint32_t attachmentIndex, const glm::vec4& clearValue)
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

	void OpenGLFrameBuffer::Cleanup()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(static_cast<uint32_t>(m_ColorAttachments.size()), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
		glDeleteSamplers(static_cast<uint32_t>(m_ColorSamplers.size()), m_ColorSamplers.data());
		glDeleteSamplers(static_cast<uint32_t>(m_DepthSamplers.size()), m_DepthSamplers.data());
		glDeleteTextures(static_cast<uint32_t>(m_DepthAttachmentViews.size()), m_DepthAttachmentViews.data());
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
