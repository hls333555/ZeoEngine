#include "Core/AssetManager.h"

#include "Core/AssetActions.h"
#include "Core/AssetFactory.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	void AssetManager::Init()
	{
		RegisterAssetActions(AssetType<Scene>::Id(), CreateRef<SceneAssetActions>());
		RegisterAssetActions(AssetType<ParticleTemplate>::Id(), CreateRef<ParticleAssetActions>());
		RegisterAssetActions(AssetType<Texture2D>::Id(), CreateRef<Texture2DAssetActions>());
	}

	bool AssetManager::RegisterAssetActions(AssetTypeId typeId, Ref<IAssetActions> actions)
	{
		return m_AssetActions.insert(std::make_pair(typeId, actions)).second;
	}

	bool AssetManager::RegisterAssetFactory(AssetTypeId typeId, Ref<AssetFactoryBase> factory)
	{
		factory->SetAssetType(typeId);
		return m_AssetFactories.insert(std::make_pair(typeId, factory)).second;
	}

	void AssetManager::CreateAsset(AssetTypeId typeId, const std::string& path) const
	{
		auto it = m_AssetFactories.find(typeId);
		if (it == m_AssetFactories.end()) return;

		it->second->CreateAsset(path);
	}

	bool AssetManager::OpenAsset(const std::string& path)
	{
		AssetTypeId typeId = FileUtils::GetAssetTypeIdFromFile(path);
		auto result = m_AssetActions.find(typeId);
		if (result != m_AssetActions.end())
		{
			result->second->OpenAsset(path);
			return true;
		}

		ZE_CORE_WARN("Failed to open asset: {0}. Unknown file format!", FileUtils::GetFileNameFromPath(path));
		return false;
	}

	Ref<IAssetActions> AssetManager::GetAssetActionsByAssetType(AssetTypeId typeId)
	{
		if (auto it = m_AssetActions.find(typeId); it != m_AssetActions.cend())
		{
			return it->second;
		}

		return {};
	}

	Ref<AssetFactoryBase> AssetManager::GetAssetFactoryByAssetType(AssetTypeId typeId)
	{
		if (auto it = m_AssetFactories.find(typeId); it != m_AssetFactories.cend())
		{
			return it->second;
		}

		return {};
	}

}
