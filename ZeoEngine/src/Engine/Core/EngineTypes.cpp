#include "ZEpch.h"
#include "Engine/Core/EngineTypes.h"

#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	AssetPath::AssetPath(const char* path)
		: m_RelativePath(PathUtils::GetRelativePath(path))
	{
	}

	AssetPath::AssetPath(const std::string& path)
		: m_RelativePath(PathUtils::GetRelativePath(path))
	{
	}

	std::string AssetPath::GetName() const
	{
		return PathUtils::GetNameFromPath(m_RelativePath);
	}

}
