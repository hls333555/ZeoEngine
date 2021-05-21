#include "Core/AssetManager.h"

#include "Core/AssetActions.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	void AssetManager::Init()
	{
		m_AssetActions[AssetType::Scene] = CreateRef<SceneAssetActions>();
		m_AssetActions[AssetType::ParticleTemplate] = CreateRef<ParticleAssetActions>();
		m_AssetActions[AssetType::Texture2D] = CreateRef<Texture2DAssetActions>();
	}

	bool AssetManager::OpenAsset(const std::string& path)
	{
		AssetType assetType = FileUtils::GetAssetTypeFromFile(path);
		auto result = m_AssetActions.find(assetType);
		if (result != m_AssetActions.end())
		{
			result->second->OpenAsset(path);
			return true;
		}

		ZE_CORE_WARN("Failed to open asset: {0}. Unknown file format!", FileUtils::GetFileNameFromPath(path));
		return false;
	}

}
