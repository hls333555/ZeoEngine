#include "ZEpch.h"
#include "Engine/Core/EngineTypes.h"

#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	const char* g_EngineAssetExtension = ".zasset";
	const char* g_AssetTypeToken = "AssetType";
	const char* g_AssetNameToken = "AssetName";

	AssetPath::AssetPath(const char* path)
		: m_RelativePath(FileUtils::GetRelativePath(path))
	{
	}
	AssetPath::AssetPath(const std::string& path)
		: m_RelativePath(FileUtils::GetRelativePath(path))
	{
	}

}
