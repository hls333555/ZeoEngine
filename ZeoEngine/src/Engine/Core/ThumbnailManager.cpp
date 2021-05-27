#include "ZEpch.h"
#include "Engine/Core/ThumbnailManager.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	namespace Utils {

		static const char* GetThumbnailCacheDirectory()
		{
			return "assets/cache/thumbnails";
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
	}

	Ref<Texture2D> ThumbnailManager::GetAssetThumbnail(const std::string& path)
	{
		std::string thumbnailPath = GetAssetThumbnailPath(path);
		return PathUtils::DoesPathExist(thumbnailPath) ? Texture2D::Create(thumbnailPath, true) : Ref<Texture2D>{};
	}

	std::string ThumbnailManager::GetAssetThumbnailPath(const std::string& assetPath)
	{
		auto pathId = entt::hashed_string{ assetPath.c_str() }.value();
		return PathUtils::AppendPath(Utils::GetThumbnailCacheDirectory(), std::to_string(pathId));
	}

}
