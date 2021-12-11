#pragma once

#include "Engine/GameFramework/Entity.h"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <deque>

#include "Engine/Core/ReflectionHelper.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	extern const char* g_AssetTypeToken;
	extern const char* g_SourceToken;

	class ComponentSerializer
	{
	public:
		void Serialize(YAML::Emitter& out, entt::meta_any& instance);
		void Deserialize(const YAML::Node& value, entt::meta_any& instance, IAsset* asset);

	private:
		/** Reverse data order so that serialized and deserialized datas are in correct order. */
		void PreprocessDatas(entt::meta_any& instance);

		void EvaluateSerializeData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement);
		void EvaluateSerializeSequenceContainerData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance);
		void EvaluateSerializeAssociativeContainerData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance);
		void EvaluateSerializeStructData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement);

		template<typename T>
		void SerializeData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
		{
			if (bIsSeqElement)
			{
				const auto elementValue = instance.cast<T>();
				if constexpr (std::is_same<T, uint8_t>::value)
				{
					// This '+' can force output uint8_t as number
					out << +elementValue;
				}
				else
				{
					out << elementValue;
				}
			}
			else
			{
				const auto dataName = GetMetaObjectDisplayName(data);
				const auto dataValue = data.get(instance).cast<T>();
				if constexpr (std::is_same<T, uint8_t>::value)
				{
					out << YAML::Key << *dataName << YAML::Value << +dataValue;
				}
				else
				{
					out << YAML::Key << *dataName << YAML::Value << dataValue;
				}
			}
		}
		void SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement);

		void EvaluateDeserializeData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement);
		void EvaluateDeserializeSequenceContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value);
		void EvaluateDeserializeAssociativeContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value);
		void EvaluateDeserializeStructData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement);

		template<typename T>
		void DeserializeData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
		{
			const auto& dataValue = value.as<T>();
			if (bIsSeqElement)
			{
				instance.cast<T&>() = dataValue;
			}
			else
			{
				data.set(instance, dataValue);
			}
		}
		void DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement);

	private:
		std::deque<entt::meta_data> m_PreprocessedDatas;
	};

	class MaterialSerializer
	{
	public:
		void Serialize(YAML::Emitter& out, const Ref<Material>& material);
		void Deserialize(const YAML::Node& value, const Ref<Material>& material);

	private:
		void EvaluateSerializeData(YAML::Emitter& out, const Ref<DynamicUniformDataBase>& uniform);

		template<typename T>
		void SerializeData(YAML::Emitter& out, const Ref<DynamicUniformDataBase>& uniform)
		{
			const auto& dataName = uniform->Name;
			const auto& dataValue = *static_cast<T*>(uniform->GetValuePtr());
			out << YAML::Key << dataName << YAML::Value << dataValue;
		}

		void EvaluateDeserializeData(const YAML::Node& value, const Ref<DynamicUniformDataBase>& uniform);

		template<typename T>
		void DeserializeData(const YAML::Node& value, const Ref<DynamicUniformDataBase>& uniform)
		{
			const auto& dataValue = value.as<T>();
			*static_cast<T*>(uniform->GetValuePtr()) = dataValue;
		}
	};

	class Serializer
	{
	public:
		/**
		 * Serialize data to asset.
		 * The signature of the func must be equivalent to the following form:
		 * 
		 * @code:
		 * void(YAML::Emitter& out);
		 * @endcode
		 *
		 */
		template<typename Func>
		static void WriteDataToAsset(const std::string& path, AssetTypeId typeId, Func func)
		{
			YAML::Emitter out;

			const std::string assetName = PathUtils::GetNameFromPath(path);
			//ZE_CORE_TRACE("Serializing \"{0}\"", assetName);

			out << YAML::BeginMap;
			{
				out << YAML::Key << g_AssetTypeToken << YAML::Value << typeId;
				func(out);
			}
			out << YAML::EndMap;

			std::ofstream fout(path);
			fout << out.c_str();
		}

		/**
		 * Deserialize data from asset.
		 * If optionalTypeId is specified, it will verify it against the one from the asset.
		 */
		static std::optional<YAML::Node> ReadDataFromAsset(const std::string& path, std::optional<AssetTypeId> optionalTypeId = {})
		{
			auto data = YAML::LoadFile(path);
			auto assetTypeData = data[g_AssetTypeToken];
			const std::string assetName = PathUtils::GetNameFromPath(path);
			if (!assetTypeData || (optionalTypeId && assetTypeData.as<AssetTypeId>() != *optionalTypeId))
			{
				ZE_CORE_ERROR("Failed to load \"{0}\". Unknown format!", assetName);
				return {};
			}
			
			//ZE_CORE_TRACE("Deserializing \"{0}\"", assetName);
			return data;
		}
	};

	class AssetSerializer : public Serializer
	{
	public:
		static void Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance);
		static bool Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, IAsset* asset);
	};

	class ImportableAssetSerializer : public AssetSerializer
	{
	public:
		static void Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const std::string& resourcePath = {});
		static bool Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, IAsset* asset);
	};

	class MaterialAssetSerializer : public AssetSerializer
	{
	public:
		static void Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const Ref<Material>& material);
		static bool Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const Ref<Material>& material, IAsset* asset);
	};

	class SceneSerializer : public Serializer
	{
	public:
		static void Serialize(const std::string& path, const Ref<Scene>& scene);
		static void SerializeRuntime();
		static bool Deserialize(const std::string& path, const Ref<Scene>& scene, IAsset* asset);
		static bool DeserializeRuntime();
	};

}
