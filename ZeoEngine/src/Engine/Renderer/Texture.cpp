#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Ref<Texture2D> Texture2D::Create(std::string ID, U32 width, U32 height, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(std::move(ID), width, height, format, bindingSlot, type);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(std::string ID, U32 hexColor, bool bIsSRGB, std::optional<U32> bindingSlot)
	{
		auto texture = Create(std::move(ID), 1, 1, bIsSRGB ? TextureFormat::SRGB8 : TextureFormat::RGB8, bindingSlot);
		texture->SetData(&hexColor, 3);
		return texture;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, std::optional<U32> bindingSlot)
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
				texture = CreateRef<OpenGLTexture2D>(std::move(resourcePath), bindingSlot);
				break;
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}

		texture->Deserialize();
		return texture;
	}

	void Texture2D::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));
		ImportableAssetSerializer::Serialize(GetID(), TypeId(), TexturePreviewComponent{ GetAssetHandle() });
	}

	void Texture2D::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		ImportableAssetSerializer::Deserialize(GetID(), TypeId(), TexturePreviewComponent{ GetAssetHandle() }, this);
	}

	Ref<Texture2DArray> Texture2DArray::Create(U32 width, U32 height, U32 arraySize, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2DArray>(width, height, arraySize, format, bindingSlot, type);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

}
