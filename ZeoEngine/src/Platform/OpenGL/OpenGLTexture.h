#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace ZeoEngine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual const std::string& GetPath() const override { return m_Path; }
		virtual bool HasAlpha() const override { return m_bHasAlpha; }
		virtual void* GetTexture() const override { return (void*)(intptr_t)m_RendererID; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

	private:
		/** Intended for hot-reloading */
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_bHasAlpha = false;
	};

}
