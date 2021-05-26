#include "Core/AssetManager.h"

#include "Core/AssetActions.h"
#include "Core/AssetFactory.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	void AssetManager::Init()
	{
		RegisterAssetFactory(AssetType<Scene>::Id(), CreateRef<SceneAssetFactory>());
		RegisterAssetFactory(AssetType<ParticleTemplate>::Id(), CreateRef<ParticleTemplateAssetFactory>());
		RegisterAssetFactory(AssetType<Texture2D>::Id(), CreateRef<Texture2DAssetFactory>());

		RegisterAssetActions(AssetType<Scene>::Id(), CreateRef<SceneAssetActions>());
		RegisterAssetActions(AssetType<ParticleTemplate>::Id(), CreateRef<ParticleAssetActions>());
		RegisterAssetActions(AssetType<Texture2D>::Id(), CreateRef<Texture2DAssetActions>());
	}

	bool AssetManager::RegisterAssetFactory(AssetTypeId typeId, Ref<IAssetFactory> factory)
	{
		return m_AssetFactories.insert(std::make_pair(typeId, factory)).second;
	}

	bool AssetManager::RegisterAssetActions(AssetTypeId typeId, Ref<IAssetActions> actions)
	{
		return m_AssetActions.insert(std::make_pair(typeId, actions)).second;
	}

	void AssetManager::CreateAsset(AssetTypeId typeId, const std::string& path) const
	{
		auto it = m_AssetFactories.find(typeId);
		if (it == m_AssetFactories.end()) return;

		it->second->CreateAsset(typeId, path);
	}

	bool AssetManager::OpenAsset(const std::string& path)
	{
		AssetTypeId typeId = AssetRegistry::Get().GetPathSpec<AssetSpec>(path)->TypeId;
		auto result = m_AssetActions.find(typeId);
		if (result != m_AssetActions.end())
		{
			result->second->OpenAsset(path);
			return true;
		}

		ZE_CORE_WARN("Failed to open asset: {0}. Unknown file format!", PathUtils::GetFileNameFromPath(path));
		return false;
	}

	Ref<IAssetFactory> AssetManager::GetAssetFactoryByAssetType(AssetTypeId typeId)
	{
		if (auto it = m_AssetFactories.find(typeId); it != m_AssetFactories.cend())
		{
			return it->second;
		}

		return {};
	}

	Ref<IAssetActions> AssetManager::GetAssetActionsByAssetType(AssetTypeId typeId)
	{
		if (auto it = m_AssetActions.find(typeId); it != m_AssetActions.cend())
		{
			return it->second;
		}

		return {};
	}

}
