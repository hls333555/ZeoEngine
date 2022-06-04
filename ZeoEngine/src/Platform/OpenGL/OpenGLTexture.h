#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace ZeoEngine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(std::string ID, U32 width, U32 height, TextureFormat format, SamplerType type);
		OpenGLTexture2D(std::string path, bool bAutoGenerateMipmaps);
		virtual ~OpenGLTexture2D() override;

		virtual U32 GetWidth() const override { return m_Width; }
		virtual U32 GetHeight() const override { return m_Height; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(U32 index) const override { return GetTextureID(); }

		virtual void SetData(void* data, U32 size) override;
		virtual void ChangeSampler(SamplerType type) override;

		virtual void Bind(U32 slot) const override;
		virtual void Unbind(U32 slot) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2D&>(other).m_RendererID;
		}

	private:
		U32 m_RendererID;
		std::string m_TextureResourcePath;

		U32 m_Width, m_Height;
		U32 m_MipmapLevels = 1;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		Ref<Sampler> m_Sampler;
	};

	class OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(U32 width, U32 height, U32 arraySize, TextureFormat format, SamplerType type);
		virtual ~OpenGLTexture2DArray() override;

		virtual U32 GetWidth() const override { return m_Width; }
		virtual U32 GetHeight() const override { return m_Height; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(U32 index = 0) const override { return (void*)(intptr_t)m_TextureViews[index]; }

		virtual void SetData(void* data, U32 size) override;
		virtual void ChangeSampler(SamplerType type) override;

		virtual void Bind(U32 slot) const override;
		virtual void Unbind(U32 slot) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2DArray&>(other).m_RendererID;
		}

	private:
		U32 m_Width, m_Height;
		U32 m_MipmapLevels = 1;
		U32 m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		Ref<Sampler> m_Sampler;

		U32 m_ArraySize;
		std::vector<U32> m_TextureViews;
	};

}
