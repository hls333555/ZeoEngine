#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace ZeoEngine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(std::string ID, U32 width, U32 height, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type);
		OpenGLTexture2D(std::string path, std::optional<U32> bindingSlot);
		virtual ~OpenGLTexture2D() override;

		virtual U32 GetWidth() const override { return m_Width; }
		virtual U32 GetHeight() const override { return m_Height; }
		virtual TextureFormat GetFormat() const override { return m_Format; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual bool bIsSRGB() const override { return m_bIsSRGB; }
		virtual void SetSRGB(bool bSRGB) override { m_bIsSRGB = bSRGB; Invalidate(); }
		virtual SamplerType GetSamplerType() const override { return m_Sampler ? m_Sampler->GetType() : SamplerType::BilinearRepeat; }
		virtual void ChangeSampler(SamplerType type) override { m_Sampler = SamplerLibrary::GetOrAddSampler(type); }
		virtual bool ShouldGenerateMipmaps() const override { return m_bShouldGenerateMipmaps; }
		virtual void SetGenerateMipmaps(bool bGenerate) override { m_bShouldGenerateMipmaps = bGenerate; Invalidate(); }
		virtual U32 GetMipmapLevels() const override { return m_MipmapLevels; }

		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(U32 index) const override { return GetTextureID(); }

		virtual void SetData(void* data, U32 size) override;

		virtual void Invalidate() override;

		virtual void SetBindingSlot(U32 slot) override { m_BindingSlot = slot; }
		virtual void Bind() const override;
		virtual void BindAsImage(U32 slot, bool bReadOrWrite) const override;
		virtual void Unbind() const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2D&>(other).m_RendererID;
		}

	private:
		U32 m_RendererID = 0;
		std::string m_TextureResourcePath;

		U32 m_Width, m_Height;
		TextureFormat m_Format = TextureFormat::None;
		bool m_bIsSRGB = true;
		bool m_bShouldGenerateMipmaps = true;
		U32 m_MipmapLevels = 1;
		std::optional<U32> m_BindingSlot;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		Ref<Sampler> m_Sampler;
	};

	class OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(U32 width, U32 height, U32 arraySize, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type);
		virtual ~OpenGLTexture2DArray() override;

		virtual U32 GetWidth() const override { return m_Width; }
		virtual U32 GetHeight() const override { return m_Height; }
		virtual TextureFormat GetFormat() const override { return m_Format; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual bool bIsSRGB() const override { return m_bIsSRGB; }
		virtual void SetSRGB(bool bSRGB) override { m_bIsSRGB = bSRGB; }
		virtual SamplerType GetSamplerType() const override { return m_Sampler->GetType(); }
		virtual void ChangeSampler(SamplerType type) override { m_Sampler = SamplerLibrary::GetOrAddSampler(type); }
		virtual bool ShouldGenerateMipmaps() const override { return m_bShouldGenerateMipmaps; }
		virtual void SetGenerateMipmaps(bool bGenerate) override { m_bShouldGenerateMipmaps = bGenerate; }
		virtual U32 GetMipmapLevels() const override { return m_MipmapLevels; }

		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(U32 index = 0) const override { return (void*)(intptr_t)m_TextureViews[index]; }

		virtual void SetData(void* data, U32 size) override;

		virtual void SetBindingSlot(U32 slot) override { m_BindingSlot = slot; }
		virtual void Bind() const override;
		virtual void BindAsImage(U32 slot, bool bReadOrWrite) const override;
		virtual void Unbind() const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2DArray&>(other).m_RendererID;
		}

	private:
		U32 m_RendererID = 0;

		U32 m_Width, m_Height;
		TextureFormat m_Format;
		bool m_bIsSRGB = true;
		bool m_bShouldGenerateMipmaps = true;
		U32 m_MipmapLevels = 1;
		std::optional<U32> m_BindingSlot;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		Ref<Sampler> m_Sampler;

		U32 m_ArraySize;
		std::vector<U32> m_TextureViews;
	};

}
