#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		ZE_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		// Allocate memory on the GPU to store the data
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool bAutoGenerateMipmaps)
		: m_Path(PathUtils::GetRelativePath(path))
		, m_FileName(PathUtils::GetFileNameFromPath(path))
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
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		ZE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		ZE_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

}
