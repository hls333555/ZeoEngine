#include "ZEpch.h"
#include "Engine/Core/ThumbnailManager.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Core/AssetManager.h"

namespace ZeoEngine {

	namespace Utils {

		static const char* GetDirectoryIconPath()
		{
			return "resources/textures/icons/Folder.png";
		}

		static const char* GetAssetTypeIconDirectory()
		{
			return "resources/textures/icons";
		}

		static std::string GetAssetTypeIconPath(AssetTypeId typeId)
		{
			std::string pathName = std::to_string(typeId) + ".png";
			return PathUtils::AppendPath(GetAssetTypeIconDirectory(), pathName);
		}

		static const char* GetThumbnailCacheDirectory()
		{
			return "cache/thumbnails";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetThumbnailCacheDirectory();
			if (!PathUtils::DoesPathExist(cacheDirectory))
			{
				PathUtils::CreateDirectories(cacheDirectory);
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
		AssetManager::Get().ForEachAssetFactory([this](AssetTypeId typeId)
		{
			const std::string thumbnailPath = Utils::GetAssetTypeIconPath(typeId);
			ZE_CORE_ASSERT(PathUtils::DoesPathExist(thumbnailPath));

			m_AssetTypeIcons[typeId] = Texture2D::Create(thumbnailPath);
		});

		m_DirectoryIcon = Texture2D::Create(Utils::GetDirectoryIconPath());
	}

	Ref<Texture2D> ThumbnailManager::GetAssetThumbnail(const std::string& path, AssetTypeId typeId)
	{
		const std::string thumbnailPath = GetAssetThumbnailPath(path, typeId);
		return PathUtils::DoesPathExist(thumbnailPath) ? Texture2DLibrary::Get().LoadAsset(thumbnailPath).to_ref() : m_AssetTypeIcons[typeId];
	}

	std::string ThumbnailManager::GetAssetThumbnailPath(const std::string& assetPath, AssetTypeId typeId) const
	{
		if (typeId == Texture2D::TypeId())
		{
			return PathUtils::GetResourcePathFromPath(assetPath);
		}

		const auto pathId = entt::hashed_string{ assetPath.c_str() }.value();
		return PathUtils::AppendPath(Utils::GetThumbnailCacheDirectory(), std::to_string(pathId));
	}

	Ref<Texture2D> ThumbnailManager::GetAssetTypeIcon(AssetTypeId typeId) const
	{
		if (auto it = m_AssetTypeIcons.find(typeId); it != m_AssetTypeIcons.cend())
		{
			return it->second;
		}

		return {};
	}

}
