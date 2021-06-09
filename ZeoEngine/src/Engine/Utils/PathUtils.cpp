#include "ZEpch.h"
#include "Engine/Utils/PathUtils.h"

#include <filesystem>

namespace ZeoEngine {

	std::string PathUtils::GetCanonicalPath(const std::string& path)
	{
		return std::filesystem::canonical(path).string();
	}

	std::string PathUtils::GetRelativePath(const std::string& path)
	{
		return std::filesystem::relative(std::filesystem::canonical(path)).string();
	}

	std::string PathUtils::GetParentPath(const std::string& path)
	{
		return std::filesystem::path{ path }.parent_path().string();
	}

	std::string PathUtils::GetFileNameFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.filename().string();
	}

	std::string PathUtils::GetNameFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.stem().string();
	}

	std::string PathUtils::GetExtensionFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.extension().string();
	}

	std::string PathUtils::AppendPath(const std::string& basePath, const std::string& appendPath)
	{
		auto path = std::filesystem::path{ basePath };
		return (path / appendPath).string();
	}

	bool PathUtils::DoesPathExist(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	bool PathUtils::CreateDirectory(const std::string& directory)
	{
		return std::filesystem::create_directory(directory);
	}

	bool PathUtils::CreateDirectories(const std::string& directory)
	{
		return std::filesystem::create_directories(directory);
	}

	void PathUtils::RenamePath(const std::string& oldPath, const std::string& newPath)
	{
		std::filesystem::rename(oldPath, newPath);
	}

	uintmax_t PathUtils::DeletePath(const std::string& path, bool bRecursively)
	{
		if (bRecursively)
		{
			return std::filesystem::remove_all(path);
		}
		else
		{
			return std::filesystem::remove(path) ? 1 : 0;
		}
	}

	bool PathUtils::CopyFile(const std::string& srcPath, const std::string& destPath, bool bShouldOverwrite)
	{
		return std::filesystem::copy_file(srcPath, destPath, bShouldOverwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none);
	}

	std::string PathUtils::GetResourcePathFromAssetPath(const std::string& assetPath)
	{
		return assetPath.substr(0, assetPath.rfind("."));
	}

}
