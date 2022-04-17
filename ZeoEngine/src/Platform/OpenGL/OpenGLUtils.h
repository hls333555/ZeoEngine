#pragma once

#include <glad/glad.h>

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class OpenGLUtils
	{
	public:
		static GLenum ToGLTextureInternalFormat(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::RGBA8:				return GL_RGBA8;
				case TextureFormat::RGBA16F:			return GL_RGBA16F;
				case TextureFormat::RED_INTEGER:		return GL_R32I;
				case TextureFormat::DEPTH32F:			return GL_DEPTH_COMPONENT32F;
				case TextureFormat::DEPTH24STENCIL8:	return GL_DEPTH24_STENCIL8;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}

		static GLenum ToGLTextureFormat(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::RGBA8:
				case TextureFormat::RGBA16F:			return GL_RGBA;
				case TextureFormat::RED_INTEGER:		return GL_RED_INTEGER;
				case TextureFormat::DEPTH32F:			return GL_DEPTH_COMPONENT;
				case TextureFormat::DEPTH24STENCIL8:	return GL_DEPTH_STENCIL;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}

		static GLenum ToGLDataType(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::RGBA8:			return GL_UNSIGNED_BYTE;
				case TextureFormat::RGBA16F:		return GL_FLOAT;
				case TextureFormat::RED_INTEGER:	return GL_INT;
				default:
					ZE_CORE_ASSERT(false);
					break;
			}
			return 0;
		}
	};

}
