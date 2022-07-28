#include "ZEpch.h"
#include "Engine/Core/ThumbnailManager.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetRegistry.h"

namespace ZeoEngine {

	namespace Utils {

		static const char* GetDirectoryIconPath()
		{
			return "resources/textures/icons/Folder.png";
		}

		static std::filesystem::path GetAssetTypeIconDirectory()
		{
			return "resources/textures/icons";
		}

		static std::filesystem::path GetAssetTypeIconPath(AssetTypeID typeID)
		{
			const std::string pathName = std::to_string(typeID) + ".png";
			return GetAssetTypeIconDirectory() / pathName;
		}

		static std::filesystem::path GetThumbnailCacheDirectory()
		{
			return "cache/thumbnails";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const auto cacheDirectory = GetThumbnailCacheDirectory();
			if (!PathUtils::Exists(cacheDirectory))
			{
				PathUtils::CreateDirectory(cacheDirectory);
			}
		}

	}

	void ThumbnailManager::Init()
	{
		Utils::CreateCacheDirectoryIfNeeded();

		LoadAssetTypeIcons();
	}

	void ThumbnailManager::LoadAssetTypeIcons()
	{
		AssetManager::Get().ForEachAssetType([this](AssetTypeID typeID)
		{
			const auto thumbnailPath = Utils::GetAssetTypeIconPath(typeID);
			ZE_CORE_ASSERT(PathUtils::Exists(thumbnailPath));

			m_AssetTypeIcons[typeID] = Texture2D::Create(thumbnailPath.string());
		});

		m_DirectoryIcon = Texture2D::Create(Utils::GetDirectoryIconPath());
	}

	Ref<Texture2D> ThumbnailManager::GetAssetThumbnail(const Ref<AssetMetadata>& metadata)
	{
		const auto thumbnailPath = GetAssetThumbnailPath(metadata);
		return PathUtils::Exists(thumbnailPath) ? Texture2D::Create(thumbnailPath.string()) : m_AssetTypeIcons[metadata->TypeID];
	}

	std::filesystem::path ThumbnailManager::GetAssetThumbnailPath(const Ref<AssetMetadata>& metadata) const
	{
		if (metadata->TypeID == Texture2D::TypeID())
		{
			return metadata->GetResourcePath();
		}

		return Utils::GetThumbnailCacheDirectory() / std::to_string(metadata->Handle);
	}

	Ref<Texture2D> ThumbnailManager::GetAssetTypeIcon(AssetTypeID typeID) const
	{
		if (const auto it = m_AssetTypeIcons.find(typeID); it != m_AssetTypeIcons.cend())
		{
			return it->second;
		}

		return {};
	}

}
