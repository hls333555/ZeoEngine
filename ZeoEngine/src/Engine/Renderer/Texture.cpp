#include "ZEpch.h"
#include "Engine/Renderer/Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	Ref<Texture2D> Texture2D::Create(U32 width, U32 height, TextureFormat format, std::optional<U32> bindingSlot, SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(width, height, format, bindingSlot, type);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(U32 hexColor, bool bIsSRGB, std::optional<U32> bindingSlot)
	{
		auto texture = Create(1, 1, bIsSRGB ? TextureFormat::SRGB8 : TextureFormat::RGB8, bindingSlot);
		texture->SetData(&hexColor, 3);
		return texture;
	}

	Ref<Texture2D> Texture2D::Create(std::string resourcePath, std::optional<U32> bindingSlot)
	{
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

		return texture;
	}

	Ref<Texture2D> Texture2D::GetWhiteTexture()
	{
		static AssetHandle handle = AssetLibrary::CreateMemoryOnlyAsset<Texture2D>(0xffffff);
		return AssetLibrary::LoadAsset<Texture2D>(handle);
	}

	Ref<Texture2D> Texture2D::GetDefaultMaterialTexture()
	{
		static AssetHandle handle = AssetLibrary::CreateMemoryOnlyAsset<Texture2D>(0x808080);
		return AssetLibrary::LoadAsset<Texture2D>(handle);
	}

	Ref<Texture2D> Texture2D::GetAssetBackgroundTexture()
	{
		static AssetHandle handle = AssetLibrary::CreateMemoryOnlyAsset<Texture2D>(0x151414);
		return AssetLibrary::LoadAsset<Texture2D>(handle);
	}

	Ref<Texture2D> Texture2D::GetCheckerboardTexture()
	{
		return AssetLibrary::LoadAsset<Texture2D>("assets/editor/textures/Checkerboard.png.zasset");
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
