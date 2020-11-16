#include "ZEpch.h"
#include "Engine/Utils/EngineUtils.h"

#include <filesystem>

namespace ZeoEngine {

	std::string GetCanonicalPath(const std::string& path)
	{
		return std::filesystem::canonical(path).string();
	}

	std::string GetRelativePath(const std::string& path)
	{
		return std::filesystem::relative(std::filesystem::canonical(path)).string();
	}

	std::string GetFileNameFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.filename().string();
	}

	std::string GetNameFromPath(const std::string& path)
	{
		return std::filesystem::path{ path }.stem().string();
	}

}
