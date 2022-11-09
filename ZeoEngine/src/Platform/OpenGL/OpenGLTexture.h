#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace ZeoEngine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(U32 width, U32 height, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type);
		OpenGLTexture2D(std::string resourcePath, std::optional<U32> bindingSlot);
		virtual ~OpenGLTexture2D() override;

		virtual U32 GetWidth() const override { return m_Width; }
		virtual U32 GetHeight() const override { return m_Height; }
		virtual TextureFormat GetFormat() const override { return m_Format; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual U32 GetMipmapLevels() const override { return m_MipmapLevels; }

		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(U32 index) const override { return GetTextureID(); }

		virtual void Invalidate() override;

		virtual void SetData(void* data, U32 size) override;

		virtual void SetBindingSlot(U32 slot) override { m_BindingSlot = slot; }
		virtual void Bind() const override;
		virtual void BindAsImage(U32 slot, bool bReadOrWrite) const override;
		virtual void Unbind() const override;

	private:
		U32 m_RendererID = 0;
		std::string m_TextureResourcePath;

		U32 m_Width, m_Height;
		TextureFormat m_Format = TextureFormat::None;
		U32 m_MipmapLevels = 1;
		std::optional<U32> m_BindingSlot;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;
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
		virtual U32 GetMipmapLevels() const override { return m_MipmapLevels; }

		virtual void* GetTextureID() const override { return (void*)(intptr_t)m_RendererID; }
		virtual void* GetTextureViewID(U32 index = 0) const override { return (void*)(intptr_t)m_TextureViews[index]; }

		virtual void Invalidate() override;

		virtual void SetData(void* data, U32 size) override;

		virtual void SetBindingSlot(U32 slot) override { m_BindingSlot = slot; }
		virtual void Bind() const override;
		virtual void BindAsImage(U32 slot, bool bReadOrWrite) const override;
		virtual void Unbind() const override;

	private:
		U32 m_RendererID = 0;

		U32 m_Width, m_Height;
		TextureFormat m_Format;
		U32 m_MipmapLevels = 1;
		std::optional<U32> m_BindingSlot;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;

		U32 m_ArraySize;
		std::vector<U32> m_TextureViews;
	};

}
