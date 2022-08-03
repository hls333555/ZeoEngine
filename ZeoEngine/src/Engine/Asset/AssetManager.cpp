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

	// https://stackoverflow.com/questions/28386185/cant-use-stdunique-ptrt-with-t-being-a-forward-declaration
	AssetManager::AssetManager() = default;
	AssetManager::~AssetManager() = default;

	void AssetManager::Init()
	{
		RegisterAssetFactory(Level::TypeID(), CreateScope<LevelAssetFactory>());
		RegisterAssetFactory(ParticleTemplate::TypeID(), CreateScope<ParticleTemplateAssetFactory>());
		RegisterAssetFactory(Texture2D::TypeID(), CreateScope<Texture2DAssetFactory>());
		RegisterAssetFactory(Mesh::TypeID(), CreateScope<MeshAssetFactory>());
		RegisterAssetFactory(Material::TypeID(), CreateScope<MaterialAssetFactory>());
		RegisterAssetFactory(Shader::TypeID(), CreateScope<ShaderAssetFactory>());

		RegisterAssetActions(Level::TypeID(), CreateScope<LevelAssetActions>());
		RegisterAssetActions(ParticleTemplate::TypeID(), CreateScope<ParticleTemplateAssetActions>());
		RegisterAssetActions(Texture2D::TypeID(), CreateScope<Texture2DAssetActions>());
		RegisterAssetActions(Mesh::TypeID(), CreateScope<MeshAssetActions>());
		RegisterAssetActions(Material::TypeID(), CreateScope<MaterialAssetActions>());
		RegisterAssetActions(Shader::TypeID(), CreateScope<ShaderAssetActions>());

		RegisterAssetSerializer(Level::TypeID(), CreateScope<LevelAssetSerializer>());
		RegisterAssetSerializer(ParticleTemplate::TypeID(), CreateScope<ParticleTemplateAssetSerializer>());
		RegisterAssetSerializer(Texture2D::TypeID(), CreateScope<Texture2DAssetSerializer>());
		RegisterAssetSerializer(Mesh::TypeID(), CreateScope<MeshAssetSerializer>());
		RegisterAssetSerializer(Material::TypeID(), CreateScope<MaterialAssetSerializer>());
		RegisterAssetSerializer(Shader::TypeID(), CreateScope<ShaderAssetSerializer>());

		InitSupportedFileExtensions();
	}

	bool AssetManager::RegisterAssetFactory(AssetTypeID typeID, Scope<AssetFactoryBase> factory)
	{
		factory->m_TypeID = typeID;
		return m_AssetFactories.insert(std::make_pair(typeID, std::move(factory))).second;
	}

	bool AssetManager::RegisterAssetActions(AssetTypeID typeID, Scope<AssetActionsBase> actions)
	{
		return m_AssetActions.insert(std::make_pair(typeID, std::move(actions))).second;
	}

	bool AssetManager::RegisterAssetSerializer(AssetTypeID typeID, Scope<AssetSerializerBase> serializer)
	{
		return m_AssetSerializers.insert(std::make_pair(typeID, std::move(serializer))).second;
	}

	static void UnknownAssetTypeWarning(const char* operation, const std::filesystem::path& path, AssetTypeID typeID)
	{
		ZE_CORE_WARN("Failed to {0} asset: {1} with unknown asset type: {2}!", operation, std::filesystem::path(path).filename(), typeID);
	}

	bool AssetManager::CreateAssetFile(AssetTypeID typeID, const std::filesystem::path& path) const
	{
		if (const auto* factory = GetAssetFactoryByAssetType(typeID))
		{
			factory->CreateAssetFile(path);
			return true;
		}

		UnknownAssetTypeWarning("create empty", path, typeID);
		return false;
	}

	bool AssetManager::ImportAsset(AssetTypeID typeID, const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const
	{
		if (const auto* factory = GetAssetFactoryByAssetType(typeID))
		{
			factory->ImportAsset(srcPath, destPath);
			return true;
		}

		UnknownAssetTypeWarning("import", srcPath, typeID);
		return false;
	}

	Ref<IAsset> AssetManager::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		if (const auto* factory = GetAssetFactoryByAssetType(metadata->TypeID))
		{
			return factory->CreateAsset(metadata);
		}

		UnknownAssetTypeWarning("create", metadata->Path, metadata->TypeID);
		return nullptr;
	}

	bool AssetManager::OpenAsset(const std::filesystem::path& path) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		if (const auto* actions = GetAssetActionsByAssetType(typeID))
		{
			actions->OpenAsset(path);
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
		if (const auto* actions = GetAssetActionsByAssetType(typeID))
		{
			actions->RenameAsset(oldPath, newPath);
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
		if (const auto* actions = GetAssetActionsByAssetType(typeID))
		{
			actions->DeleteAsset(path);
			return true;
		}

		UnknownAssetTypeWarning("delete", path, typeID);
		return false;
	}

	bool AssetManager::SaveAsset(const std::filesystem::path& path, const Ref<IAsset>& asset) const
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		if (!metadata) return false;

		const auto typeID = metadata->TypeID;
		if (const auto* serializer = GetAssetSerializerByAssetType(typeID))
		{
			serializer->Serialize(metadata, asset);
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
		if (const auto* actions = GetAssetActionsByAssetType(typeID))
		{
			actions->ReimportAsset(path);
			return true;
		}

		UnknownAssetTypeWarning("reimport", path, typeID);
		return false;
	}

	AssetFactoryBase* AssetManager::GetAssetFactoryByAssetType(AssetTypeID typeID) const
	{
		if (const auto it = m_AssetFactories.find(typeID); it != m_AssetFactories.cend())
		{
			return it->second.get();
		}

		return nullptr;
	}

	AssetActionsBase* AssetManager::GetAssetActionsByAssetType(AssetTypeID typeID) const
	{
		if (const auto it = m_AssetActions.find(typeID); it != m_AssetActions.cend())
		{
			return it->second.get();
		}

		return nullptr;
	}

	AssetSerializerBase* AssetManager::GetAssetSerializerByAssetType(AssetTypeID typeID) const
	{
		if (const auto it = m_AssetSerializers.find(typeID); it != m_AssetSerializers.cend())
		{
			return it->second.get();
		}

		return nullptr;
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
