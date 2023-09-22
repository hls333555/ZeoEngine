#include "ZEpch.h"
#include "Engine/Asset/AssetSerializer.h"

#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Material.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Utils/SceneUtils.h"

namespace ZeoEngine {

	void AssetSerializerBase::Serialize(const AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
		SerializeEmptyAsset(metadata->Path, metadata->TypeID, metadata->Handle, metadata->Flags, true, [&](YAML::Node& node)
		{
			SerializeImpl(metadata, asset, node);
		});
	}

	bool AssetSerializerBase::Deserialize(AssetMetadata* metadata, const Ref<IAsset>& asset, void* payload) const
	{
		const auto res = DeserializeAsset(metadata->Path);
		if (!res) return false;

		const auto& node = *res;
		const auto typeID = node["AssetType"].as<AssetTypeID>();
		const auto handle = node["AssetHandle"].as<AssetHandle>();
		ZE_CORE_ASSERT(typeID == metadata->TypeID && handle == metadata->Handle);

		return DeserializeImpl(metadata, asset, node, payload);
	}

	std::optional<YAML::Node> AssetSerializerBase::DeserializeAsset(const std::string& path)
	{
		YAML::Node node;
		try
		{
			node = YAML::LoadFile(FileSystemUtils::GetFileSystemPath(path));
		}
		catch (YAML::BadFile&)
		{
			ZE_CORE_ERROR("Failed to load asset: {0}!", path);
			return {};
		}

		const auto assetTypeData = node["AssetType"];
		const auto assetHandleData = node["AssetHandle"];
		if (!assetTypeData || !assetHandleData)
		{
			ZE_CORE_ERROR("Failed to load asset: {0} with invalid data!", path);
			return {};
		}
		return node;
	}

	void ImportableAssetSerializerBase::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		SerializeSourcePath(metadata->SourcePath, node);
	}

	bool ImportableAssetSerializerBase::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		if (const auto sourceData = node["SourcePath"])
		{
			metadata->SourcePath = sourceData.as<std::string>();
		}
		return true;
	}

	void ImportableAssetSerializerBase::SerializeSourcePath(const std::string& resourcePath, YAML::Node& node)
	{
		node["SourcePath"] = resourcePath;
	}

#pragma region LevelAssetSerializer
	void LevelAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const Ref<Level> level = std::static_pointer_cast<Level>(asset);
		Scene* scene = level->GetScene();
		SceneSerializer::Serialize(node, *scene);
	}

	bool LevelAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const Ref<Level> level = std::static_pointer_cast<Level>(asset);
		auto* scene = static_cast<Scene*>(payload);
		level->SetScene(scene);
		SceneSerializer::Deserialize(node, *scene);
		return true;
	}

	void LevelAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
		SceneUtils::OpenLevel(metadata->Path);
	}
#pragma endregion

#pragma region ParticleTemplateAssetSerializer
	void ParticleTemplateAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const auto particleTemplate = std::static_pointer_cast<ParticleTemplate>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, ParticleSystemDetailComponent(particleTemplate));
	}

	bool ParticleTemplateAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const auto particleTemplate = std::static_pointer_cast<ParticleTemplate>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, ParticleSystemDetailComponent(particleTemplate), this);
		return true;
	}

	void ParticleTemplateAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
		const auto particleTemplate = std::static_pointer_cast<ParticleTemplate>(asset);
		Deserialize(metadata, asset, nullptr);
		particleTemplate->ResimulateAllParticleSystemInstances();
	}
#pragma endregion

#pragma region Texture2DAssetSerializer
	void Texture2DAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		ImportableAssetSerializerBase::SerializeImpl(metadata, asset, node);

		const auto texture = std::static_pointer_cast<Texture2D>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, TextureDetailComponent(texture));
	}

	bool Texture2DAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		ImportableAssetSerializerBase::DeserializeImpl(metadata, asset, node, payload);

		const auto texture = std::static_pointer_cast<Texture2D>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, TextureDetailComponent(texture), this);
		return true;
	}

	void Texture2DAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
		const auto texture = std::static_pointer_cast<Texture2D>(asset);
		texture->Invalidate();
		Deserialize(metadata, asset, nullptr);
	}
#pragma endregion

#pragma region MeshAssetSerializer
	void MeshAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		ImportableAssetSerializerBase::SerializeImpl(metadata, asset, node);

		const auto mesh = std::static_pointer_cast<Mesh>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, MeshDetailComponent(mesh));
	}

	bool MeshAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		ImportableAssetSerializerBase::DeserializeImpl(metadata, asset, node, payload);

		const auto mesh = std::static_pointer_cast<Mesh>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, MeshDetailComponent(mesh), this);
		return true;
	}

	void MeshAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{

	}
#pragma endregion

#pragma region ShaderAssetSerializer
	void ShaderAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		
	}

	bool ShaderAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		return true;
	}

	void ShaderAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
		const auto shader = std::static_pointer_cast<Shader>(asset);
		shader->Reload();
	}
#pragma endregion

#pragma region MaterialAssetSerializer
	void MaterialAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const auto material = std::static_pointer_cast<Material>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, MaterialDetailComponent(material));
		MaterialSerializer ms;
		ms.Serialize(node, material);
	}

	bool MaterialAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const auto material = std::static_pointer_cast<Material>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, MaterialDetailComponent(material), this);
		return true;
	}

	void MaterialAssetSerializer::PostFieldDeserialize(IComponent* comp, U32 fieldID) const
	{
		const auto* materialComp = static_cast<MaterialDetailComponent*>(comp);
		if (fieldID == "ShaderAsset"_hs)
		{
			materialComp->LoadedMaterial->NotifyShaderAssetChange();
		}
	}

	void MaterialAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
		Deserialize(metadata, asset, nullptr);
	}

	void MaterialAssetSerializer::DeserializeShaderData(const AssetMetadata* metadata, const Ref<Material>& material) const
	{
		const auto res = DeserializeAsset(metadata->Path);
		if (!res) return;
		
		const auto& node = *res;
		MaterialSerializer ms;
		ms.Deserialize(node, material);
		material->ApplyDynamicFields();
	}
#pragma endregion

#pragma region PhysicsMaterialAssetSerializer
	void PhysicsMaterialAssetSerializer::SerializeImpl(const AssetMetadata* metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const auto physicsMaterial = std::static_pointer_cast<PhysicsMaterial>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, PhysicsMaterialDetailComponent(physicsMaterial));
	}

	bool PhysicsMaterialAssetSerializer::DeserializeImpl(AssetMetadata* metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const auto physicsMaterial = std::static_pointer_cast<PhysicsMaterial>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, PhysicsMaterialDetailComponent(physicsMaterial), this);
		return true;
	}

	void PhysicsMaterialAssetSerializer::ReloadData(AssetMetadata* metadata, const Ref<IAsset>& asset) const
	{
	}
#pragma endregion

}
