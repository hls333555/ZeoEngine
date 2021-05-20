#include "Core/AssetManager.h"

#include "Core/AssetActions.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	void AssetManager::Init()
	{
		m_AssetActions[".zscene"] = CreateRef<SceneAssetActions>();
		m_AssetActions[".zparticle"] = CreateRef<ParticleAssetActions>();
	}

	bool AssetManager::OpenAsset(const std::string& path)
	{
		std::string extension = GetExtensionFromPath(path);
		auto result = m_AssetActions.find(extension);
		if (result != m_AssetActions.end())
		{
			result->second->OpenAsset(path);
			return true;
		}

		ZE_CORE_WARN("Failed to open asset: {0}. Unknown extension!", GetFileNameFromPath(path));
		return false;
	}

}
