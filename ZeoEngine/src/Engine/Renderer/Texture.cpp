#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Ref<Texture2D> Texture2D::Create(std::string ID, uint32_t width, uint32_t height, TextureFormat format, SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(std::move(ID), width, height, format, type);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(std::string ID, uint32_t hexColor)
	{
		auto texture = Create(std::move(ID), 1, 1, TextureFormat::RGB8);
		texture->SetData(&hexColor, 3);
		return texture;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool bAutoGenerateMipmaps)
	{
		std::string resourcePath = PathUtils::GetResourcePathFromPath(path);
		if (!PathUtils::DoesPathExist(resourcePath)) return {};

		Ref<Texture2D> texture;
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				texture = CreateRef<OpenGLTexture2D>(std::move(resourcePath), bAutoGenerateMipmaps);
				break;
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}

		texture->Deserialize();
		return texture;
	}

	// TODO:
	void Texture2D::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));
		ImportableAssetSerializer::Serialize(GetID(), TypeId(), {});
	}

	// TODO:
	void Texture2D::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		ImportableAssetSerializer::Deserialize(GetID(), TypeId(), {}, this);
	}

	AssetHandle<Texture2D> Texture2DLibrary::GetWhiteTexture()
	{
		return Get().LoadAsset("ZID_WhiteTexture", 0xffffff);
	}

	AssetHandle<Texture2D> Texture2DLibrary::GetDefaultMaterialTexture()
	{
		return Get().LoadAsset("ZID_DefaultMaterialTexture", 0x808080);
	}

	AssetHandle<Texture2D> Texture2DLibrary::GetAssetBackgroundTexture()
	{
		return Get().LoadAsset("ZID_AssetBackgroundTexture", 0x151414);
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

}
