#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include <filesystem>

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace ZeoEngine {

	std::string GetCanonicalPath(const std::string& path)
	{
		return std::filesystem::canonical(path).string();
	}

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
		const auto canonicalPath = GetCanonicalPath(path);
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			ZE_CORE_ASSERT_INFO(false, "RendererAPI is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(canonicalPath);
		default:
			ZE_CORE_ASSERT_INFO(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	void Texture2DLibrary::Add(const std::string& path, const Ref<Texture2D>& texture)
	{
		if (!Exists(path))
		{
			m_Textures[GetCanonicalPath(path)] = texture;
		}
	}

	void Texture2DLibrary::Add(const Ref<Texture2D>& texture)
	{
		const std::string& path = texture->GetPath();
		Add(path, texture);
	}

	Ref<Texture2D> Texture2DLibrary::Load(const std::string& path)
	{
		auto texture = Texture2D::Create(path);
		Add(texture);
		return texture;
	}

	Ref<Texture2D> Texture2DLibrary::GetOrLoad(const std::string& path)
	{
		if (Exists(path))
		{
			return m_Textures[GetCanonicalPath(path)];
		}
		else
		{
			return Load(path);
		}
	}

	Ref<Texture2D> Texture2DLibrary::Get(const std::string& path)
	{
		ZE_CORE_ASSERT_INFO(Exists(path), "Texture not found!");
		return m_Textures[GetCanonicalPath(path)];
	}

	bool Texture2DLibrary::Exists(const std::string& path) const
	{
		return m_Textures.find(GetCanonicalPath(path)) != m_Textures.end();
	}

}
