#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace ZeoEngine {
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	void Texture2DLibrary::Add(const std::string& path, const Ref<Texture2D>& texture)
	{
		ZE_CORE_ASSERT_INFO(!Exists(path), "Trying to add the texture which already exists!");
		m_Textures[path] = texture;
	}

	void Texture2DLibrary::Add(const Ref<Texture2D>& texture)
	{
		const std::string& path = texture->GetPath();
		Add(path, texture);
	}

	Ref<Texture2D> Texture2DLibrary::Load(const std::string& filePath)
	{
		auto texture = Texture2D::Create(filePath);
		Add(texture);
		return texture;
	}

	Ref<Texture2D> Texture2DLibrary::Load(const std::string& path, const std::string& filePath)
	{
		auto texture = Texture2D::Create(filePath);
		Add(path, texture);
		return texture;
	}

	Ref<Texture2D> Texture2DLibrary::Get(const std::string& path)
	{
		ZE_CORE_ASSERT_INFO(Exists(path), "Texture not found!");
		return m_Textures[path];
	}

	bool Texture2DLibrary::Exists(const std::string& path) const
	{
		return m_Textures.find(path) != m_Textures.end();
	}

}
