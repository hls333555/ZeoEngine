#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	Ref<Texture2D> Texture2D::s_DefaultBackgroundTexture;

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(width, height);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool bAutoGenerateMipmaps)
	{
		const auto canonicalPath = PathUtils::GetRelativePath(path);
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(canonicalPath, bAutoGenerateMipmaps);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Texture2DAsset::Texture2DAsset(const std::string& path)
		: AssetImpl(path)
	{
		m_TexturePath = GetTexturePath(path);
		ZE_CORE_ASSERT(PathUtils::DoesPathExist(m_TexturePath));
		m_Texture = Texture2D::Create(m_TexturePath);
		Deserialize(); // Do not call it in constructor if it contains shared_from_this()
	}

	Ref<Texture2DAsset> Texture2DAsset::Create(const std::string& path)
	{
		// A way to allow std::make_shared() to access Texture2DAsset's private constructor
		class Texture2DAssetEnableShared : public Texture2DAsset
		{
		public:
			explicit Texture2DAssetEnableShared(const std::string& path)
				: Texture2DAsset(path) {}
		};

		return CreateRef<Texture2DAssetEnableShared>(path);
	}

	std::string Texture2DAsset::GetTexturePath(const std::string& path)
	{
		auto pos = path.rfind(".");
		return path.substr(0, pos);
	}

}
