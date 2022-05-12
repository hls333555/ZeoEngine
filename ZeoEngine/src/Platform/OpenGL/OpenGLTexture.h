#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace ZeoEngine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(std::string ID, uint32_t width, uint32_t height, TextureFormat format, SamplerType type);
		OpenGLTexture2D(std::string path, bool bAutoGenerateMipmaps);
		virtual ~OpenGLTexture2D() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(uint32_t index) const override { return GetTextureID(); }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void ChangeSampler(SamplerType type) override;

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind(uint32_t slot) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2D&>(other).m_RendererID;
		}

	private:
		uint32_t m_Width, m_Height;
		uint32_t m_MipmapLevels = 1;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		Ref<Sampler> m_Sampler;
	};

	class OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(uint32_t width, uint32_t height, uint32_t arraySize, TextureFormat format, SamplerType type);
		virtual ~OpenGLTexture2DArray() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(uint32_t index = 0) const override { return (void*)(intptr_t)m_TextureViews[index]; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void ChangeSampler(SamplerType type) override;

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind(uint32_t slot) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2DArray&>(other).m_RendererID;
		}

	private:
		uint32_t m_Width, m_Height;
		uint32_t m_MipmapLevels = 1;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		Ref<Sampler> m_Sampler;

		uint32_t m_ArraySize;
		std::vector<uint32_t> m_TextureViews;
	};

}
