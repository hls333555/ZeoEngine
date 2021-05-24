#include "ZEpch.h"
#include "Engine/Utils/EngineUtils.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>

namespace ZeoEngine {

	std::string FileUtils::GetCanonicalPath(const std::string& path)
	{
		return std::filesystem::canonical(path).string();
	}

	std::string FileUtils::GetRelativePath(const std::string& path)
	{
		return std::filesystem::relative(std::filesystem::canonical(path)).string();
	}

	std::string FileUtils::GetFileNameFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.filename().string();
	}

	std::string FileUtils::GetNameFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.stem().string();
	}

	std::string FileUtils::GetExtensionFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.extension().string();
	}

	AssetTypeId FileUtils::GetAssetTypeIdFromFile(const std::string& path)
	{
		std::string extension = GetExtensionFromPath(path);
		ZE_CORE_ASSERT(extension == g_EngineAssetExtension);

		auto data = YAML::LoadFile(path);
		auto assetTypeData = data[g_AssetTypeToken];
		if (!assetTypeData) return g_InvalidAssetType;

		AssetTypeId typeId = assetTypeData.as<AssetTypeId>();
		return typeId;
	}

}
