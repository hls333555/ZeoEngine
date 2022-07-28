#pragma once

#include <optional>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	class IAsset;
	struct AssetMetadata;
	class Scene;

	class IAssetSerializer
	{
	public:
		virtual ~IAssetSerializer() = default;

		virtual void Serialize(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const = 0;
		virtual bool Deserialize(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const = 0;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const = 0;
	};

	class AssetSerializerBase : public IAssetSerializer
	{
	public:
		virtual void Serialize(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;
		virtual bool Deserialize(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;

		template<typename Func>
		static void SerializeEmptyAsset(const std::filesystem::path& path, AssetTypeID typeID, AssetHandle handle, bool bOverwrite, Func func)
		{
			YAML::Node node;
			const auto f = [&]()
			{
				node["AssetType"] = typeID;
				node["AssetHandle"] = handle;
				func(node);
				std::ofstream fout(path);
				fout << node;
			};
			if (!bOverwrite)
			{
				try
				{
					node = YAML::LoadFile(path.string());
				}
				catch (YAML::BadFile&)
				{
				}
			}
			f();
		}
		static void SerializeEmptyAsset(const std::filesystem::path& path, AssetTypeID typeID, AssetHandle handle, bool bOverwrite)
		{
			SerializeEmptyAsset(path, typeID, handle, bOverwrite, [](YAML::Node& node) {});
		}
		static std::optional<YAML::Node> DeserializeAsset(const std::filesystem::path& path);

	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const = 0;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const = 0;
	};

	class ImportableAssetSerializerBase : public AssetSerializerBase
	{
	public:
		template<typename Func>
		static void SerializeEmptyAsset(const std::filesystem::path& path, const std::filesystem::path& resourcePath, AssetTypeID typeID, AssetHandle handle, bool bOverwrite, Func func)
		{
			AssetSerializerBase::SerializeEmptyAsset(path, typeID, handle, bOverwrite, [&](YAML::Node& node)
			{
				SerializeSourcePath(resourcePath, node);
				func(node);
			});
		}
		static void SerializeEmptyAsset(const std::filesystem::path& path, const std::filesystem::path& resourcePath, AssetTypeID typeID, AssetHandle handle, bool bOverwrite)
		{
			SerializeEmptyAsset(path, resourcePath, typeID, handle, bOverwrite, [](YAML::Node& node) {});
		}

	protected:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		static void SerializeSourcePath(const std::filesystem::path& resourcePath, YAML::Node& node);
	};

	class LevelAssetSerializer : public AssetSerializerBase
	{
	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;
	};

	class ParticleTemplateAssetSerializer : public AssetSerializerBase
	{
	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;
	};

	class Texture2DAssetSerializer : public ImportableAssetSerializerBase
	{
	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;
	};

	class MeshAssetSerializer : public ImportableAssetSerializerBase
	{
	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;
	};

	class ShaderAssetSerializer : public AssetSerializerBase
	{
	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;
	};

	class MaterialAssetSerializer : public AssetSerializerBase
	{
	public:
		void ReloadShaderData(const Ref<AssetMetadata>& metadata, const Ref<Material>& material) const;

	private:
		virtual void SerializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, YAML::Node& node) const override;
		virtual bool DeserializeImpl(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset, const YAML::Node& node) const override;
		virtual void ReloadData(const Ref<AssetMetadata>& metadata, const Ref<IAsset>& asset) const override;

		bool DeserializeImplInternal(const Ref<AssetMetadata>& metadata, const Ref<Material>& material, const YAML::Node& node, bool bIncludeComponentData) const;
	};
	
}
