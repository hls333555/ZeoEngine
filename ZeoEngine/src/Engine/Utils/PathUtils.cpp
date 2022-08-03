#include "ZEpch.h"
#include "Engine/Utils/PathUtils.h"

#include <filesystem>

#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetSerializer.h"

namespace ZeoEngine {

	bool PathUtils::Exists(const std::filesystem::path& path)
	{
		return std::filesystem::exists(path);
	}

	bool PathUtils::IsDirectory(const std::filesystem::path& path)
	{
		return std::filesystem::is_directory(path);
	}

	bool PathUtils::Equivalent(const std::filesystem::path& lPath, const std::filesystem::path& rPath)
	{
		return std::filesystem::equivalent(lPath, rPath);
	}

	bool PathUtils::CreateDirectory(const std::filesystem::path& directory)
	{
		return std::filesystem::create_directories(directory);
	}

	void PathUtils::RenamePath(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
	{
		std::filesystem::rename(oldPath, newPath);
	}

	bool PathUtils::DeletePath(const std::filesystem::path& path)
	{
		if (!Exists(path)) return false;

		if (IsDirectory(path))
		{
			return std::filesystem::remove_all(path) > 0;
		}
		return std::filesystem::remove(path);
	}

	bool PathUtils::CopyFile(const std::filesystem::path& srcPath, const std::filesystem::path& destPath, bool bShouldOverwrite)
	{
		return std::filesystem::copy_file(srcPath, destPath, bShouldOverwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none);
	}

	bool PathUtils::CopyAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath)
	{
		// No need to copy self
		if (Exists(destPath) && Equivalent(srcPath, destPath)) return false;

		const auto metadata = AssetRegistry::Get().GetAssetMetadata(srcPath);
		if (!metadata) return false;

		if (metadata->IsResourceAsset())
		{
			// Copy resource
			const bool bSuccess = CopyFile(metadata->GetResourcePath(), destPath.parent_path() / destPath.stem(), true);
			if (!bSuccess)
			{
				ZE_CORE_ERROR("Failed to copy resource file!");
				return false;
			}
		}

		// Copy asset
		const bool bSuccess = CopyFile(srcPath, destPath, true);
		if (!bSuccess)
		{
			ZE_CORE_ERROR("Failed to copy asset file!");
			return false;
		}

		// Assign new asset handle to the copied asset
		AssetSerializerBase::SerializeEmptyAsset(destPath, metadata->TypeID, AssetHandle(), false);

		return true;
	}

}
