#pragma once

#include <optional>
#include <string>

namespace ZeoEngine {

	enum class AssetType
	{
		NONE,
		Scene,
		ParticleTemplate,
		Texture,
	};

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
