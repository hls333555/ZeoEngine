#pragma once

#include <optional>
#include <string>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static std::optional<std::string> OpenFile(AssetType type);
		static std::optional<std::string> SaveFile(AssetType type);

	private:
		static const char* GetAssetFilterFromAssetType(AssetType type);
	};

}
