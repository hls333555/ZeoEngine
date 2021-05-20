#pragma once

#include "Engine/GameFramework/Entity.h"

#include <yaml-cpp/yaml.h>
#include <magic_enum.hpp>

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
		template<typename Func>
		static void Serialize_t(const std::string& path, AssetType assetType, Func func)
		{
			YAML::Emitter out;

			auto assetTypeName = magic_enum::enum_name(assetType).data();
			const std::string assetName = GetNameFromPath(path);
			ZE_CORE_TRACE("Serializing {0} '{1}'", assetTypeName, assetName);

			out << YAML::BeginMap;
			{
				out << YAML::Key << assetTypeName << YAML::Value << assetName;
				func(out);
			}
			out << YAML::EndMap;

			std::ofstream fout(path);
			fout << out.c_str();
		}

		static std::optional<YAML::Node> PreDeserialize(const std::string& path, AssetType assetType);
	};

	class AssetSerializer : public Serializer
	{
	public:
		static void Serialize(const std::string& path, AssetType assetType, entt::meta_any instance);
		static bool Deserialize(const std::string& path, AssetType assetType, entt::meta_any instance);
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
