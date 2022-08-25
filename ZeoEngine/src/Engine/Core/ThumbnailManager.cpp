#include "ZEpch.h"
#include "Engine/Core/ThumbnailManager.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetRegistry.h"

namespace ZeoEngine {

	namespace Utils {

		// TODO: Move
		static std::string GetDirectoryIconPath()
		{
			return "resources/textures/icons/Folder.png";
		}

		static std::string GetAssetTypeIconDirectory()
		{
			return "resources/textures/icons";
		}

		static std::string GetAssetTypeIconPath(AssetTypeID typeID)
		{
			const std::string pathName = std::to_string(typeID) + ".png";
			return fmt::format("{}/{}", GetAssetTypeIconDirectory(), pathName);
		}

		static std::string GetThumbnailCacheDirectory()
		{
			return "cache/thumbnails";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const std::string cacheDirectory = GetThumbnailCacheDirectory();
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
			std::string thumbnailPath = Utils::GetAssetTypeIconPath(typeID);
			ZE_CORE_ASSERT(PathUtils::Exists(thumbnailPath));

			m_AssetTypeIcons[typeID] = Texture2D::Create(std::move(thumbnailPath));
		});

		m_DirectoryIcon = Texture2D::Create(Utils::GetDirectoryIconPath());
	}

	Ref<Texture2D> ThumbnailManager::GetAssetThumbnail(const Ref<AssetMetadata>& metadata)
	{
		std::string thumbnailPath = GetAssetThumbnailPath(metadata);
		return PathUtils::Exists(thumbnailPath) ? Texture2D::Create(std::move(thumbnailPath)) : m_AssetTypeIcons[metadata->TypeID];
	}

	std::string ThumbnailManager::GetAssetThumbnailPath(const Ref<AssetMetadata>& metadata) const
	{
		if (metadata->TypeID == Texture2D::TypeID())
		{
			return metadata->GetResourceFileSystemPath();
		}

		return fmt::format("{}/{}", Utils::GetThumbnailCacheDirectory(), std::to_string(metadata->Handle));
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
