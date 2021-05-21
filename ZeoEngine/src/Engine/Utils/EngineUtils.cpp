#include "ZEpch.h"
#include "Engine/Utils/EngineUtils.h"

#include <yaml-cpp/yaml.h>
#include <magic_enum.hpp>

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

	AssetType FileUtils::GetAssetTypeFromFile(const std::string& path)
	{
		std::string extension = GetExtensionFromPath(path);
		ZE_CORE_ASSERT(extension == g_EngineAssetExtension);

		auto data = YAML::LoadFile(path);
		auto assetTypeData = data[g_AssetTypeToken];
		if (!assetTypeData) return AssetType::NONE;

		std::string assetTypeStr = assetTypeData.as<std::string>();
		auto result = magic_enum::enum_cast<AssetType>(assetTypeStr);
		return result ? *result : AssetType::NONE;
	}

}
