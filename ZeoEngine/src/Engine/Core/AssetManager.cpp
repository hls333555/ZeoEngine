#include "ZEpch.h"
#include "Engine/Core/AssetManager.h"

#include "Engine/Core/AssetActions.h"
#include "Engine/Core/AssetFactory.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	void AssetManager::Init()
	{
		RegisterAssetFactory(SceneAsset::TypeId(), CreateRef<SceneAssetFactory>());
		RegisterAssetFactory(ParticleTemplateAsset::TypeId(), CreateRef<ParticleTemplateAssetFactory>());
		RegisterAssetFactory(Texture2DAsset::TypeId(), CreateRef<Texture2DAssetFactory>());

		RegisterAssetActions(SceneAsset::TypeId(), CreateRef<SceneAssetActions>());
		RegisterAssetActions(ParticleTemplateAsset::TypeId(), CreateRef<ParticleAssetActions>());
		RegisterAssetActions(Texture2DAsset::TypeId(), CreateRef<Texture2DAssetActions>());

		InitSupportedFileExtensions();
	}

	bool AssetManager::RegisterAssetFactory(AssetTypeId typeId, Ref<IAssetFactory> factory)
	{
		std::dynamic_pointer_cast<AssetFactoryBase>(factory)->m_TypeId = typeId;
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

		it->second->CreateAsset(path);
	}

	void AssetManager::ImportAsset(AssetTypeId typeId, const std::string& srcPath, const std::string& destPath) const
	{
		auto it = m_AssetFactories.find(typeId);
		if (it == m_AssetFactories.end()) return;

		it->second->ImportAsset(srcPath, destPath);
	}

	bool AssetManager::OpenAsset(const std::string& path)
	{
		AssetTypeId typeId = AssetRegistry::Get().GetPathSpec(path)->GetAssetTypeId();
		auto result = m_AssetActions.find(typeId);
		if (result != m_AssetActions.end())
		{
			result->second->OpenAsset(path);
			return true;
		}

		ZE_CORE_WARN("Failed to open asset: {0}. Unknown file format!", PathUtils::GetFileNameFromPath(path));
		return false;
	}

	bool AssetManager::SaveAsset(const std::string& path)
	{
		AssetTypeId typeId = AssetRegistry::Get().GetPathSpec(path)->GetAssetTypeId();
		auto result = m_AssetActions.find(typeId);
		if (result != m_AssetActions.end())
		{
			result->second->SaveAsset(path);
			return true;
		}

		ZE_CORE_WARN("Failed to save asset: {0}. Unknown file format!", PathUtils::GetFileNameFromPath(path));
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

	std::optional<AssetTypeId> AssetManager::GetTypdIdFromFileExtension(const std::string& extension)
	{
		if (auto it = m_SupportedFileExtensions.find(extension); it != m_SupportedFileExtensions.cend())
		{
			return it->second;
		}

		return {};
	}

	void AssetManager::InitSupportedFileExtensions()
	{
		m_SupportedFileExtensions.insert(std::make_pair(".png", Texture2DAsset::TypeId()));
		m_SupportedFileExtensions.insert(std::make_pair(".tga", Texture2DAsset::TypeId()));
	}

}
