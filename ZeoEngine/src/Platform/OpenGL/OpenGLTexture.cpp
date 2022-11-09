#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

#include "Platform/OpenGL/OpenGLUtils.h"

namespace ZeoEngine {

	OpenGLTexture2D::OpenGLTexture2D(U32 width, U32 height, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type)
		: m_Width(width), m_Height(height)
		, m_Format(format)
		, m_BindingSlot(bindingSlot)
	{
		m_InternalFormat = OpenGLUtils::ToGLTextureInternalFormat(format);
		m_DataFormat = OpenGLUtils::ToGLTextureFormat(format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		// Allocate memory on the GPU to store the data
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (type != SamplerType::None)
		{
			ChangeSampler(type);
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(std::string resourcePath, std::optional<U32> bindingSlot)
		: m_TextureResourcePath(std::move(resourcePath))
		, m_BindingSlot(bindingSlot)
	{
		Invalidate();
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Invalidate()
	{
		if (m_RendererID) // NOTE: Make sure this is initialized to 0!
		{
			glDeleteTextures(1, &m_RendererID);
		}

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			data = stbi_load(m_TextureResourcePath.c_str(), &width, &height, &channels, 0);
		}
		ZE_CORE_ASSERT(data, "Failed to load image: {0}!", m_TextureResourcePath);
		m_Width = width;
		m_Height = height;

		switch (channels)
		{
		case 1:
			m_Format = TextureFormat::R8;
			break;
		case 3:
			m_Format = IsSRGB() ? TextureFormat::SRGB8 : TextureFormat::RGB8;
			break;
		case 4:
			m_Format = IsSRGB() ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
			m_bHasAlpha = true;
			break;
		default:
			ZE_CORE_ASSERT(m_Format != TextureFormat::None, "Texture format not supported!");
		}

		m_InternalFormat = OpenGLUtils::ToGLTextureInternalFormat(m_Format);
		m_DataFormat = OpenGLUtils::ToGLTextureFormat(m_Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

		m_MipmapLevels = static_cast<U32>(floor(log2(std::max(width, height))));
		if (!ShouldGenerateMipmaps() || m_MipmapLevels == 0)
		{
			m_MipmapLevels = 1;
		}

		// Allocate memory on the GPU to store the data
		glTextureStorage2D(m_RendererID, m_MipmapLevels, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_MipmapLevels == 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

		if (m_MipmapLevels > 1)
		{
			glGenerateTextureMipmap(m_RendererID);
		}

		stbi_image_free(data);
	}

	void OpenGLTexture2D::SetData(void* data, U32 size)
	{
		// Bytes per pixel
		U32 bpp = 0;
		switch (m_DataFormat)
		{
			case GL_RGB:	bpp = 3; break;
			case GL_RGBA:	bpp = 4; break;
		}
		ZE_CORE_ASSERT(bpp != 0, "Data format must be RGB or RGBA!");
		ZE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind() const
	{
		if (!m_BindingSlot)
		{
			ZE_CORE_ERROR("Failed to bind Texture2D ({0}, {1}) with unspecified binding slot!", m_RendererID, m_TextureResourcePath);
			return;
		}

		glBindTextureUnit(*m_BindingSlot, m_RendererID);
		if (const auto& sampler = GetSampler())
		{
			sampler->Bind(*m_BindingSlot);
		}
	}

	void OpenGLTexture2D::BindAsImage(U32 slot, bool bReadOrWrite) const
	{
		glBindImageTexture(slot, m_RendererID, 0, GL_FALSE, 0, bReadOrWrite ? GL_READ_ONLY : GL_WRITE_ONLY, m_InternalFormat);
	}

	void OpenGLTexture2D::Unbind() const
	{
		if (!m_BindingSlot)
		{
			ZE_CORE_ERROR("Failed to unbind Texture2D ({0}, {1}) with unspecified binding slot!", m_RendererID, m_TextureResourcePath);
			return;
		}

		glBindTextureUnit(*m_BindingSlot, 0);
		if (const auto& sampler = GetSampler())
		{
			sampler->Unbind(*m_BindingSlot);
		}
	}

	OpenGLTexture2DArray::OpenGLTexture2DArray(U32 width, U32 height, U32 arraySize, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type)
		: m_Width(width)
		, m_Height(height)
		, m_ArraySize(arraySize)
		, m_Format(format)
		, m_BindingSlot(bindingSlot)
	{
		m_InternalFormat = OpenGLUtils::ToGLTextureInternalFormat(format);
		m_DataFormat = OpenGLUtils::ToGLTextureFormat(format);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		// Allocate memory on the GPU to store the data
		// TODO: Generate mipmaps
		glTextureStorage3D(m_RendererID, 1, m_InternalFormat, width, height, arraySize);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (type != SamplerType::None)
		{
			ChangeSampler(type);
		}

		m_TextureViews.resize(arraySize);
		glGenTextures(arraySize, m_TextureViews.data());
		for (U32 i = 0; i < arraySize; ++i)
		{
			glTextureView(m_TextureViews[i], GL_TEXTURE_2D, m_RendererID, m_InternalFormat, 0, 1, i, 1);
		}
	}

	OpenGLTexture2DArray::~OpenGLTexture2DArray()
	{
		glDeleteTextures(1, &m_RendererID);
		glDeleteTextures(m_ArraySize, m_TextureViews.data());
	}

	void OpenGLTexture2DArray::Invalidate()
	{
		// TODO:
	}

	// TODO:
	void OpenGLTexture2DArray::SetData(void* data, U32 size)
	{
		// Bytes per pixel
		U32 bpp = 0;
		switch (m_DataFormat)
		{
			case GL_RGB:	bpp = 3; break;
			case GL_RGBA:	bpp = 4; break;
		}
		ZE_CORE_ASSERT(bpp != 0, "Data format must be RGB or RGBA!");
		ZE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2DArray::Bind() const
	{
		if (!m_BindingSlot)
		{
			ZE_CORE_ERROR("Failed to bind Texture2DArray ({0}) with unspecified binding slot!", m_RendererID);
			return;
		}

		glBindTextureUnit(*m_BindingSlot, m_RendererID);
		if (const auto& sampler = GetSampler())
		{
			sampler->Bind(*m_BindingSlot);
		}
	}

	void OpenGLTexture2DArray::BindAsImage(U32 slot, bool bReadOrWrite) const
	{
		glBindImageTexture(slot, m_RendererID, 0, GL_FALSE, 0, bReadOrWrite ? GL_READ_ONLY : GL_WRITE_ONLY, m_InternalFormat);
	}

	void OpenGLTexture2DArray::Unbind() const
	{
		if (!m_BindingSlot)
		{
			ZE_CORE_ERROR("Failed to unbind Texture2DArray ({0}) with unspecified binding slot!", m_RendererID);
			return;
		}

		glBindTextureUnit(*m_BindingSlot, 0);
		if (const auto& sampler = GetSampler())
		{
			sampler->Unbind(*m_BindingSlot);
		}
	}

}
