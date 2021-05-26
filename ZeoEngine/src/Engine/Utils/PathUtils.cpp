#include "ZEpch.h"
#include "Engine/Utils/PathUtils.h"

#include <yaml-cpp/yaml.h>
#include <filesystem>

#include "Engine/Core/AssetRegistry.h"

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

	bool PathUtils::CreateDirectory(const std::string& directory)
	{
		return std::filesystem::create_directory(directory);
	}

	void PathUtils::CreateEmptyAsset(AssetTypeId typeId, const std::string& path)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		{
			out << YAML::Key << g_AssetTypeToken << YAML::Value << typeId;
		}
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
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

	std::optional<AssetTypeId> PathUtils::ParseAssetTypeIdFromFile(const std::string& path)
	{
		std::string extension = GetExtensionFromPath(path);
		ZE_CORE_ASSERT(extension == AssetRegistry::GetEngineAssetExtension());

		auto data = YAML::LoadFile(path);
		auto assetTypeData = data[g_AssetTypeToken];
		if (!assetTypeData) return {};

		AssetTypeId typeId = assetTypeData.as<AssetTypeId>();
		return typeId;
	}

}
