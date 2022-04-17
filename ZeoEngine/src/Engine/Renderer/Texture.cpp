#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format, SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(width, height, format, type);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool bAutoGenerateMipmaps)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(path, bAutoGenerateMipmaps);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::GetDefaultTexture()
	{
		static Ref<Texture2D> texture = Texture2D::Create("assets/editor/textures/DefaultGrey.png");
		return texture;
	}

	Ref<Texture2D> Texture2D::GetAssetBackgroundTexture()
	{
		static Ref<Texture2D> texture = Texture2D::Create("resources/textures/AssetBackground.png", true);
		return texture;
	}

	Ref<Texture2DArray> Texture2DArray::Create(uint32_t width, uint32_t height, uint32_t arraySize, TextureFormat format, SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2DArray>(width, height, arraySize, format, type);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Texture2DAsset::Texture2DAsset(const std::string& path)
		: AssetBase(path)
	{
		Reload(true);
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

	void Texture2DAsset::Reload(bool bIsCreate)
	{
		auto texturePath = PathUtils::GetResourcePathFromAssetPath(GetPath());
		ZE_CORE_ASSERT(PathUtils::DoesPathExist(texturePath));
		m_Texture = Texture2D::Create(texturePath);
		Deserialize(); // Do not call it in constructor if it contains shared_from_this()
	}

	void Texture2DAsset::Serialize(const std::string& path)
	{
		if (path.empty()) return;

		if (path != GetPath())
		{
			SetPath(path);
		}
		ImportableAssetSerializer::Serialize(GetPath(), TypeId(), {}); // TODO: Update component instance here
	}

	void Texture2DAsset::Deserialize()
	{
		if (GetPath().empty()) return;

		ImportableAssetSerializer::Deserialize(GetPath(), TypeId(), {}, this);  // TODO: Update component instance here
	}

}
