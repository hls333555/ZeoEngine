#pragma once

#include "Engine/GameFramework/Entity.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Engine/Core/ReflectionHelper.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class ComponentSerializer
	{
	public:
		void Serialize(YAML::Emitter& out, entt::meta_any& instance);
		void Deserialize(const YAML::Node& value, entt::meta_any& instance);

	private:
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
	};

	class Serializer
	{
	protected:
		template<typename AssetClass, typename Func>
		static void Serialize_t(const std::string& path, Func func)
		{
			YAML::Emitter out;

			const std::string assetName = FileUtils::GetNameFromPath(path);
			ZE_CORE_TRACE("Serializing {0} '{1}'", AssetType<AssetClass>::Name(), assetName);

			out << YAML::BeginMap;
			{
				out << YAML::Key << g_AssetTypeToken << YAML::Value << AssetType<AssetClass>::Id();
				func(out);
			}
			out << YAML::EndMap;

			std::ofstream fout(path);
			fout << out.c_str();
		}

		template<typename AssetClass>
		static std::optional<YAML::Node> PreDeserialize(const std::string& path)
		{
			auto data = YAML::LoadFile(path);
			auto assetTypeData = data[g_AssetTypeToken];
			auto typeName = AssetType<AssetClass>::Name();
			if (!assetTypeData || assetTypeData.as<AssetTypeId>() != AssetType<AssetClass>::Id())
			{
				const std::string assetFileName = FileUtils::GetFileNameFromPath(path);
				ZE_CORE_ERROR("Failed to load {0}. Unknown {1} format!", assetFileName, typeName);
				return {};
			}

			const std::string assetName = FileUtils::GetNameFromPath(path);
			ZE_CORE_TRACE("Deserializing {0} '{1}'", typeName, assetName);
			return data;
		}
	};

	class AssetSerializer : public Serializer
	{
	public:
		template<typename AssetClass>
		static void Serialize(const std::string& path, entt::meta_any instance)
		{
			Serialize_t<AssetClass>(path, [&](YAML::Emitter& out)
			{
				ComponentSerializer cs;
				cs.Serialize(out, instance);
			});
		}

		template<typename AssetClass>
		static bool Deserialize(const std::string& path, entt::meta_any instance)
		{
			auto data = PreDeserialize<AssetClass>(path);
			if (!data) return false;

			ComponentSerializer cs;
			cs.Deserialize(*data, instance);
			return true;
		}
	};

	class SceneSerializer : public Serializer
	{
	public:
		static void Serialize(const std::string& path, const Ref<Scene>& scene);
		static void SerializeRuntime();
		static bool Deserialize(const std::string& path, const Ref<Scene>& scene);
		static bool DeserializeRuntime();
	};

}
