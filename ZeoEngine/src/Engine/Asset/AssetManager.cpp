#include "ZEpch.h"
#include "Engine/Asset/AssetManager.h"

#include "Engine/Asset/AssetFactory.h"
#include "Engine/Asset/AssetActions.h"
#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	std::unordered_map<AssetHandle, Ref<IAsset>> AssetLibrary::s_LoadedAssets;
	std::unordered_map<AssetHandle, Ref<IAsset>> AssetLibrary::s_MemoryAssets;

	void AssetManager::Init()
	{
		RegisterAssetFactory(Level::TypeID(), CreateRef<LevelAssetFactory>());
		RegisterAssetFactory(ParticleTemplate::TypeID(), CreateRef<ParticleTemplateAssetFactory>());
		RegisterAssetFactory(Texture2D::TypeID(), CreateRef<Texture2DAssetFactory>());
		RegisterAssetFactory(Mesh::TypeID(), CreateRef<MeshAssetFactory>());
		RegisterAssetFactory(Material::TypeID(), CreateRef<MaterialAssetFactory>());
		RegisterAssetFactory(Shader::TypeID(), CreateRef<ShaderAssetFactory>());

		RegisterAssetActions(Level::TypeID(), CreateRef<LevelAssetActions>());
		RegisterAssetActions(ParticleTemplate::TypeID(), CreateRef<ParticleTemplateAssetActions>());
		RegisterAssetActions(Texture2D::TypeID(), CreateRef<Texture2DAssetActions>());
		RegisterAssetActions(Mesh::TypeID(), CreateRef<MeshAssetActions>());
		RegisterAssetActions(Material::TypeID(), CreateRef<MaterialAssetActions>());
		RegisterAssetActions(Shader::TypeID(), CreateRef<ShaderAssetActions>());

		RegisterAssetSerializer(Level::TypeID(), CreateRef<LevelAssetSerializer>());
		RegisterAssetSerializer(ParticleTemplate::TypeID(), CreateRef<ParticleTemplateAssetSerializer>());
		RegisterAssetSerializer(Texture2D::TypeID(), CreateRef<Texture2DAssetSerializer>());
		RegisterAssetSerializer(Mesh::TypeID(), CreateRef<MeshAssetSerializer>());
		RegisterAssetSerializer(Material::TypeID(), CreateRef<MaterialAssetSerializer>());
		RegisterAssetSerializer(Shader::TypeID(), CreateRef<ShaderAssetSerializer>());

		InitSupportedFileExtensions();
	}

	bool AssetManager::RegisterAssetFactory(AssetTypeID typeID, Ref<AssetFactoryBase> factory)
	{
		factory->m_TypeID = typeID;
		return m_AssetFactories.insert(std::make_pair(typeID, std::move(factory))).second;
	}

	bool AssetManager::RegisterAssetActions(AssetTypeID typeID, Ref<AssetActionsBase> actions)
	{
		return m_AssetActions.insert(std::make_pair(typeID, std::move(actions))).second;
	}

	bool AssetManager::RegisterAssetSerializer(AssetTypeID typeID, Ref<AssetSerializerBase> serializer)
	{
		return m_AssetSerializers.insert(std::make_pair(typeID, std::move(serializer))).second;
	}

	static void UnknownAssetTypeWarning(const char* operation, const std::filesystem::path& path, AssetTypeID typeID)
	{
		ZE_CORE_WARN("Failed to {0} asset: {1} with unknown asset type: {2}!", operation, std::filesystem::path(path).filename(), typeID);
	}

	bool AssetManager::CreateAssetFile(AssetTypeID typeID, const std::filesystem::path& path) const
	{
		const auto it = m_AssetFactories.find(typeID);
		if (it != m_AssetFactories.end())
		{
			it->second->CreateAssetFile(path);
			return true;
		}

		UnknownAssetTypeWarning("create empty", path, typeID);
		return false;
	}

	bool AssetManager::ImportAsset(AssetTypeID typeID, const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const
	{
		const auto it = m_AssetFactories.find(typeID);
		if (it != m_AssetFactories.end())
		{
			it->second->ImportAsset(srcPath, destPath);
			return true;
		}

		UnknownAssetTypeWarning("import", srcPath, typeID);
		return false;
	}

	Ref<IAsset> AssetManager::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		const auto it = m_AssetFactories.find(metadata->TypeID);
		Ref<IAsset> asset;
		if (it != m_AssetFactories.end())
		{
			asset = it->second->CreateAsset(metadata);
		}
		if (!asset)
		{
			UnknownAssetTypeWarning("create", metadata->Path, metadata->TypeID);
		}
		return asset;
	}

	bool AssetManager::OpenAsset(const std::filesystem::path& path) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		const auto it = m_AssetActions.find(typeID);
		if (it != m_AssetActions.end())
		{
			it->second->OpenAsset(path);
			return true;
		}

		UnknownAssetTypeWarning("open", path, typeID);
		return false;
	}

	bool AssetManager::RenameAsset(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(oldPath);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		const auto it = m_AssetActions.find(typeID);
		if (it != m_AssetActions.end())
		{
			it->second->RenameAsset(oldPath, newPath);
			return true;
		}

		UnknownAssetTypeWarning("rename", oldPath, typeID);
		return false;
	}

	bool AssetManager::DeleteAsset(const std::filesystem::path& path) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		const auto it = m_AssetActions.find(typeID);
		if (it != m_AssetActions.end())
		{
			it->second->DeleteAsset(path);
			return true;
		}

		UnknownAssetTypeWarning("delete", path, typeID);
		return false;
	}

	bool AssetManager::SaveAsset(const std::filesystem::path& path) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		const auto it = m_AssetSerializers.find(typeID);
		if (it != m_AssetSerializers.end())
		{
			const auto asset = AssetLibrary::LoadAsset<IAsset>(path);
			it->second->Serialize(metadata, asset);
			return true;
		}

		UnknownAssetTypeWarning("save", path, typeID);
		return false;
	}

	bool AssetManager::ReimportAsset(const std::filesystem::path& path) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		const auto it = m_AssetActions.find(typeID);
		if (it != m_AssetActions.end())
		{
			it->second->ReimportAsset(path);
			return true;
		}

		UnknownAssetTypeWarning("reimport", path, typeID);
		return false;
	}

	Ref<AssetFactoryBase> AssetManager::GetAssetFactoryByAssetType(AssetTypeID typeID)
	{
		if (const auto it = m_AssetFactories.find(typeID); it != m_AssetFactories.cend())
		{
			return it->second;
		}

		return {};
	}

	Ref<AssetActionsBase> AssetManager::GetAssetActionsByAssetType(AssetTypeID typeID)
	{
		if (const auto it = m_AssetActions.find(typeID); it != m_AssetActions.cend())
		{
			return it->second;
		}

		return {};
	}

	Ref<AssetSerializerBase> AssetManager::GetAssetSerializerByAssetType(AssetTypeID typeID)
	{
		if (const auto it = m_AssetSerializers.find(typeID); it != m_AssetSerializers.cend())
		{
			return it->second;
		}

		return {};
	}

	AssetTypeID AssetManager::GetAssetTypeFromFileExtension(const std::string& extension) const
	{
		if (const auto it = m_SupportedFileExtensions.find(extension); it != m_SupportedFileExtensions.cend())
		{
			return it->second;
		}

		return {};
	}

	void AssetManager::InitSupportedFileExtensions()
	{
		m_SupportedFileExtensions.insert(std::make_pair(".png", Texture2D::TypeID()));
		m_SupportedFileExtensions.insert(std::make_pair(".tga", Texture2D::TypeID()));
		m_SupportedFileExtensions.insert(std::make_pair(".fbx", Mesh::TypeID()));
		m_SupportedFileExtensions.insert(std::make_pair(".obj", Mesh::TypeID()));
	}

}
