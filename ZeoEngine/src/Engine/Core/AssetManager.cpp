#include "ZEpch.h"
#include "Engine/Core/AssetManager.h"

#include "Engine/Core/AssetActions.h"
#include "Engine/Core/AssetFactory.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	namespace Utils {

		static const char* GetFolderIconPath()
		{
			return "assets/editor/textures/icons/Folder.png";
		}

		static const char* GetAssetTypeIconDirectory()
		{
			return "assets/editor/textures/icons";
		}

		static std::string GetAssetTypeIconPath(AssetTypeId typeId)
		{
			std::string pathName = std::to_string(typeId) + ".png";
			return PathUtils::AppendPath(GetAssetTypeIconDirectory(), pathName);
		}

	}

	void AssetManager::Init()
	{
		RegisterAssetFactory(SceneAsset::TypeId(), CreateRef<SceneAssetFactory>());
		RegisterAssetFactory(ParticleTemplateAsset::TypeId(), CreateRef<ParticleTemplateAssetFactory>());
		RegisterAssetFactory(Texture2DAsset::TypeId(), CreateRef<Texture2DAssetFactory>());

		RegisterAssetActions(SceneAsset::TypeId(), CreateRef<SceneAssetActions>());
		RegisterAssetActions(ParticleTemplateAsset::TypeId(), CreateRef<ParticleAssetActions>());
		RegisterAssetActions(Texture2DAsset::TypeId(), CreateRef<Texture2DAssetActions>());

		LoadAssetTypeIcons();
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

	Ref<Texture2D> AssetManager::GetAssetTypeIcon(AssetTypeId typeId) const
	{
		if (auto it = m_AssetTypeIcons.find(typeId); it != m_AssetTypeIcons.cend())
		{
			return it->second;
		}

		return {};
	}

	void AssetManager::LoadAssetTypeIcons()
	{
		for (const auto& [typeId, factory] : m_AssetFactories)
		{
			std::string thumbnailPath = Utils::GetAssetTypeIconPath(typeId);
			ZE_CORE_ASSERT(PathUtils::DoesPathExist(thumbnailPath));

			m_AssetTypeIcons[typeId] = Texture2D::Create(thumbnailPath, true);
		}

		m_FolderIcon = Texture2D::Create(Utils::GetFolderIconPath(), true);
	}

}
