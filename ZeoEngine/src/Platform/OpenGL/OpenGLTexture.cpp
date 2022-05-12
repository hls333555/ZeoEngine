#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

#include "Engine/Utils/PathUtils.h"
#include "Platform/OpenGL/OpenGLUtils.h"

namespace ZeoEngine {

	OpenGLTexture2D::OpenGLTexture2D(std::string ID, uint32_t width, uint32_t height, TextureFormat format, SamplerType type)
		: Texture2D(std::move(ID)), m_Width(width), m_Height(height)
	{
		ZE_PROFILE_FUNCTION();

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
			m_Sampler = SamplerLibrary::GetOrAddSampler(type);
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(std::string path, bool bAutoGenerateMipmaps)
		: Texture2D(PathUtils::GetNormalizedAssetPath(path))
	{
		ZE_PROFILE_FUNCTION();

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			ZE_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&, bool)");

			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		ZE_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		switch (channels)
		{
			case 1:
				internalFormat = GL_R8;
				dataFormat = GL_RED;
				break;
			case 3:
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
				break;
			case 4:
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
				m_bHasAlpha = true;
				break;
			default:
				ZE_CORE_ASSERT(internalFormat & dataFormat, "Texture format not supported!");
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

		m_MipmapLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))));
		if (!bAutoGenerateMipmaps || m_MipmapLevels == 0)
		{
			m_MipmapLevels = 1;
		}

		// Allocate memory on the GPU to store the data
		glTextureStorage2D(m_RendererID, m_MipmapLevels, m_InternalFormat, m_Width, m_Height);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_MipmapLevels == 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

		if (m_MipmapLevels > 1)
		{
			glGenerateTextureMipmap(m_RendererID);
		}

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		ZE_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		ZE_PROFILE_FUNCTION();

		// Bytes per pixel
		uint32_t bpp = 0;
		switch (m_DataFormat)
		{
			case GL_RGB:	bpp = 3; break;
			case GL_RGBA:	bpp = 4; break;
		}
		ZE_CORE_ASSERT(bpp != 0, "Data format must be RGB or RGBA!");
		ZE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::ChangeSampler(SamplerType type)
	{
		m_Sampler = SamplerLibrary::GetOrAddSampler(type);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		ZE_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
		if (m_Sampler)
		{
			m_Sampler->Bind(slot);
		}
	}

	void OpenGLTexture2D::Unbind(uint32_t slot) const
	{
		ZE_PROFILE_FUNCTION();

		glBindTextureUnit(slot, 0);
		if (m_Sampler)
		{
			m_Sampler->Unbind(slot);
		}
	}

	OpenGLTexture2DArray::OpenGLTexture2DArray(uint32_t width, uint32_t height, uint32_t arraySize, TextureFormat format, SamplerType type)
		: m_ArraySize(arraySize)
	{
		ZE_PROFILE_FUNCTION();

		m_Width = width;
		m_Height = height;

		m_InternalFormat = OpenGLUtils::ToGLTextureInternalFormat(format);
		m_DataFormat = OpenGLUtils::ToGLTextureFormat(format);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		// Allocate memory on the GPU to store the data
		glTextureStorage3D(m_RendererID, 1, m_InternalFormat, width, height, arraySize);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (type != SamplerType::None)
		{
			m_Sampler = SamplerLibrary::GetOrAddSampler(type);
		}

		m_TextureViews.resize(arraySize);
		glGenTextures(arraySize, m_TextureViews.data());
		for (uint32_t i = 0; i < arraySize; ++i)
		{
			glTextureView(m_TextureViews[i], GL_TEXTURE_2D, m_RendererID, m_DataFormat, 0, 1, i, 1);
		}
	}

	OpenGLTexture2DArray::~OpenGLTexture2DArray()
	{
		ZE_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
		glDeleteTextures(m_ArraySize, m_TextureViews.data());
	}

	// TODO:
	void OpenGLTexture2DArray::SetData(void* data, uint32_t size)
	{
		ZE_PROFILE_FUNCTION();

		// Bytes per pixel
		uint32_t bpp = 0;
		switch (m_DataFormat)
		{
			case GL_RGB:	bpp = 3; break;
			case GL_RGBA:	bpp = 4; break;
		}
		ZE_CORE_ASSERT(bpp != 0, "Data format must be RGB or RGBA!");
		ZE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2DArray::ChangeSampler(SamplerType type)
	{
		m_Sampler = SamplerLibrary::GetOrAddSampler(type);
	}

	void OpenGLTexture2DArray::Bind(uint32_t slot) const
	{
		ZE_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
		if (m_Sampler)
		{
			m_Sampler->Bind(slot);
		}
	}

	void OpenGLTexture2DArray::Unbind(uint32_t slot) const
	{
		ZE_PROFILE_FUNCTION();

		glBindTextureUnit(slot, 0);
		if (m_Sampler)
		{
			m_Sampler->Unbind(slot);
		}
	}

}
