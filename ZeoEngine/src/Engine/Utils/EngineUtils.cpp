#include "ZEpch.h"
#include "Engine/Utils/EngineUtils.h"

#include <filesystem>

namespace ZeoEngine {

	std::string GetCanonicalPath(const std::string& path)
	{
		return std::filesystem::canonical(path).string();
	}

}
