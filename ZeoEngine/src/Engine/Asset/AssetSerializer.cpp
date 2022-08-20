#include "ZEpch.h"
#include "Engine/Asset/AssetSerializer.h"

#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/Material.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Utils/SceneUtils.h"

namespace ZeoEngine {

	void AssetSerializerBase::Serialize(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{
		SerializeEmptyAsset(metadata->Path, metadata->TypeID, metadata->Handle, true, [&](YAML::Node& node)
		{
			SerializeImpl(metadata, asset, node);
		});
	}

	bool AssetSerializerBase::Deserialize(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, void* payload) const
	{
		const auto res = DeserializeAsset(metadata->Path);
		if (!res) return false;

		const auto node = *res;
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
			node = YAML::LoadFile(PathUtils::GetFileSystemPath(path));
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

	void ImportableAssetSerializerBase::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		SerializeSourcePath(metadata->SourcePath, node);
	}

	bool ImportableAssetSerializerBase::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
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
	void LevelAssetSerializer::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const Ref<Level> level = std::dynamic_pointer_cast<Level>(asset);
		const auto& scene = level->GetScene();
		SceneSerializer::Serialize(node, scene);
	}

	bool LevelAssetSerializer::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const Ref<Level> level = std::dynamic_pointer_cast<Level>(asset);
		const auto& scene = *static_cast<Ref<Scene>*>(payload);
		level->SetScene(scene);
		SceneSerializer::Deserialize(node, scene);
		return true;
	}

	void LevelAssetSerializer::ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{
		SceneUtils::OpenLevel(metadata->Path);
	}
#pragma endregion

#pragma region ParticleTemplateAssetSerializer
	void ParticleTemplateAssetSerializer::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const auto particleTemplate = std::dynamic_pointer_cast<ParticleTemplate>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, ParticleSystemPreviewComponent(particleTemplate));
	}

	bool ParticleTemplateAssetSerializer::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const auto particleTemplate = std::dynamic_pointer_cast<ParticleTemplate>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, ParticleSystemPreviewComponent(particleTemplate));
		return true;
	}

	void ParticleTemplateAssetSerializer::ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{
		const auto particleTemplate = std::dynamic_pointer_cast<ParticleTemplate>(asset);
		Deserialize(metadata, asset, nullptr);
		particleTemplate->ResimulateAllParticleSystemInstances();
	}
#pragma endregion

#pragma region Texture2DAssetSerializer
	void Texture2DAssetSerializer::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		ImportableAssetSerializerBase::SerializeImpl(metadata, asset, node);

		const auto texture = std::dynamic_pointer_cast<Texture2D>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, TexturePreviewComponent(texture));
	}

	bool Texture2DAssetSerializer::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		ImportableAssetSerializerBase::DeserializeImpl(metadata, asset, node, payload);

		const auto texture = std::dynamic_pointer_cast<Texture2D>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, TexturePreviewComponent(texture));
		return true;
	}

	void Texture2DAssetSerializer::ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{
		const auto texture = std::dynamic_pointer_cast<Texture2D>(asset);
		texture->Invalidate();
		Deserialize(metadata, asset, nullptr);
	}
#pragma endregion

#pragma region MeshAssetSerializer
	void MeshAssetSerializer::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		ImportableAssetSerializerBase::SerializeImpl(metadata, asset, node);

		const auto mesh = std::dynamic_pointer_cast<Mesh>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, MeshPreviewComponent(mesh));
	}

	bool MeshAssetSerializer::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		ImportableAssetSerializerBase::DeserializeImpl(metadata, asset, node, payload);

		const auto mesh = std::dynamic_pointer_cast<Mesh>(asset);
		ComponentSerializer cs;
		if (auto* meshComp = static_cast<MeshPreviewComponent*>(payload))
		{
			// When load from Mesh Editor, the payload is passed as MeshPreviewComponent
			meshComp->MeshAsset = mesh;
			cs.Deserialize(node, *meshComp);
		}
		else
		{
			// When load from SceneSerializer, we just create a temporary MeshPreviewComponent for deserialization needs
			// Note that the PostDataDeserialize function will never get invoked as temporary MeshPreviewComponent does not create a component helper
			cs.Deserialize(node, MeshPreviewComponent(mesh));
		}
		return true;
	}

	void MeshAssetSerializer::ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{

	}
#pragma endregion

#pragma region ShaderAssetSerializer
	void ShaderAssetSerializer::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		
	}

	bool ShaderAssetSerializer::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		return true;
	}

	void ShaderAssetSerializer::ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{
		const auto shader = std::dynamic_pointer_cast<Shader>(asset);
		shader->Reload();
	}
#pragma endregion

#pragma region MaterialAssetSerializer
	void MaterialAssetSerializer::SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const
	{
		const auto material = std::dynamic_pointer_cast<Material>(asset);
		ComponentSerializer cs;
		cs.Serialize(node, MaterialPreviewComponent(material));
		MaterialSerializer ms;
		ms.Serialize(node, material);
	}

	bool MaterialAssetSerializer::DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node, void* payload) const
	{
		const auto material = std::dynamic_pointer_cast<Material>(asset);
		ComponentSerializer cs;
		cs.Deserialize(node, MaterialPreviewComponent(material));
		return true;
	}

	void MaterialAssetSerializer::ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const
	{
		Deserialize(metadata, asset, nullptr);
	}

	void MaterialAssetSerializer::DeserializeShaderData(const Ref<AssetMetadata>& metadata, const Ref<Material>& material) const
	{
		const auto res = DeserializeAsset(metadata->Path);
		if (!res) return;

		const auto node = *res;
		MaterialSerializer ms;
		ms.Deserialize(node, material);
		material->ApplyDynamicData();
	}
#pragma endregion

}
