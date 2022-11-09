#include "ZEpch.h"
#include "Engine/Utils/PathUtils.h"

#include <filesystem>

#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetSerializer.h"

namespace ZeoEngine {

	bool PathUtils::Exists(const std::string& path)
	{
		return std::filesystem::exists(GetFileSystemPath(path));
	}

	bool PathUtils::IsDirectory(const std::string& path)
	{
		return std::filesystem::is_directory(GetFileSystemPath(path));
	}

	std::string PathUtils::GetParentPath(const std::string& path)
	{
		return std::filesystem::path(path).parent_path().string();
	}

	std::string PathUtils::GetPathName(const std::string& path)
	{
		return std::filesystem::path(path).stem().string();
	}

	std::string PathUtils::GetPathFileName(const std::string& path)
	{
		return std::filesystem::path(path).filename().string();
	}

	std::string PathUtils::GetPathExtension(const std::string& path)
	{
		return std::filesystem::path(path).extension().string();
	}

	std::string PathUtils::GetCanonicalPath(const std::string& path)
	{
		return std::filesystem::weakly_canonical(GetFileSystemPath(path)).string();
	}

	bool PathUtils::CreateDirectory(const std::string& directory)
	{
		return std::filesystem::create_directories(GetFileSystemPath(directory));
	}

	void PathUtils::RenamePath(const std::string& oldPath, const std::string& newPath)
	{
		std::filesystem::rename(GetFileSystemPath(oldPath), GetFileSystemPath(newPath));
	}

	bool PathUtils::DeletePath(const std::string& path)
	{
		if (!Exists(path)) return false;

		if (IsDirectory(path))
		{
			return std::filesystem::remove_all(GetFileSystemPath(path)) > 0;
		}
		return std::filesystem::remove(GetFileSystemPath(path));
	}

	bool PathUtils::CopyFile(const std::string& srcPath, const std::string& destPath, bool bShouldOverwrite)
	{
		return std::filesystem::copy_file(GetFileSystemPath(srcPath), GetFileSystemPath(destPath), bShouldOverwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none);
	}

	bool PathUtils::CopyAsset(const std::string& srcPath, const std::string& destPath)
	{
		// No need to copy self
		if (Exists(destPath) && srcPath == destPath) return false;

		const auto metadata = AssetRegistry::Get().GetAssetMetadata(srcPath);
		if (!metadata) return false;

		if (metadata->IsResourceAsset())
		{
			// Copy resource
			const bool bSuccess = CopyFile(metadata->GetResourceFileSystemPath(), fmt::format("{}/{}", GetParentPath(destPath), GetPathName(destPath)), true);
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

	std::string PathUtils::GetStandardPath(const std::filesystem::path& path)
	{
		std::string outPath = path.is_absolute() ? std::filesystem::relative(path).generic_string() : path.generic_string();
		const std::string projectDir = AssetRegistry::GetProjectAssetDirectory();
		auto it = outPath.find(projectDir);
		if (it == 0)
		{
			outPath.replace(it, projectDir.size(), AssetRegistry::GetProjectPathPrefix());
		}
		else
		{
			const auto engineDir = AssetRegistry::GetEngineAssetDirectory();
			it = outPath.find(engineDir);
			if (it == 0)
			{
				outPath.replace(it, engineDir.size(), AssetRegistry::GetEnginePathPrefix());
			}
		}
		return outPath;
	}

	std::string PathUtils::GetFileSystemPath(const std::string& path)
	{
		std::string outPath = path;
		const std::string projectPrefix = AssetRegistry::GetProjectPathPrefix();
		auto it = outPath.find(projectPrefix);
		if (it == 0)
		{
			outPath.replace(it, projectPrefix.size(), AssetRegistry::GetProjectAssetDirectory());
		}
		else
		{
			const auto enginePrefix = AssetRegistry::GetEnginePathPrefix();
			it = outPath.find(enginePrefix);
			if (it == 0)
			{
				outPath.replace(it, enginePrefix.size(), AssetRegistry::GetEngineAssetDirectory());
			}
		}
		return outPath;
	}
}
