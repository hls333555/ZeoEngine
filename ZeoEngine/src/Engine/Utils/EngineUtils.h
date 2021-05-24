#pragma once

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class FileUtils
	{
	public:
		/** Returns absolute path. */
		static std::string GetCanonicalPath(const std::string& path);
		/** Returns relative path. */
		static std::string GetRelativePath(const std::string& path);
		/** Returns file name including file extension. */
		static std::string GetFileNameFromPath(const std::string& path);
		/** Returns file name excluding file extension. */
		static std::string GetNameFromPath(const std::string& path);
		/** Returns file extension. */
		static std::string GetExtensionFromPath(const std::string& path);

		/** Parse file and retrieve its type. */
		static AssetTypeId GetAssetTypeIdFromFile(const std::string& path);
	};

}
