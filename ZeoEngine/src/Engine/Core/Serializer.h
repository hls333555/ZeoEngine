#pragma once

#include "Engine/GameFramework/Entity.h"

#include <yaml-cpp/yaml.h>
#include <magic_enum.hpp>

#include "Engine/Core/ReflectionHelper.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	class TypeSerializer
	{
	public:
		TypeSerializer(const std::string& filePath);

		void Serialize(entt::meta_any instance, AssetType assetType);

	protected:
		template<typename Func>
		void Serialize_t(AssetType assetType, Func fn)
		{
			YAML::Emitter out;

			auto assetTypeName = magic_enum::enum_name(assetType).data();
			const std::string assetName = GetNameFromPath(m_Path);
			ZE_CORE_TRACE("Serializing {0} '{1}'", assetTypeName, assetName);

			out << YAML::BeginMap;
			{
				out << YAML::Key << assetTypeName << YAML::Value << assetName;
				fn(out);
			}
			out << YAML::EndMap;

			std::ofstream fout(m_Path);
			fout << out.c_str();
		}
		void SerializeType(YAML::Emitter& out, entt::meta_any instance);

	private:
		void EvaluateSerializeSequenceContainerData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void EvaluateSerializeAssociativeContainerData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void EvaluateSerializeNestedData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer);
		void EvaluateSerializeData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer);

		void EvaluateSerializeIntegralData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer);
		void EvaluateSerializeFloatingPointData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer);
		void EvaluateSerializeOtherData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer);

		template<typename T>
		void SerializeData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
		{
			if (bIsSeqContainer)
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
				const auto dataValue = GetDataValue<T>(data, instance);
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
		void SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer);

	public:
		bool Deserialize(entt::meta_any instance, AssetType assetType);

	protected:
		std::optional<YAML::Node> PreDeserialize(AssetType assetType);
		void DeserializeType(entt::meta_any& instance, const YAML::Node& value);

	private:
		void EvaluateDeserializeSequenceContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value);
		void EvaluateDeserializeAssociativeContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value);
		void EvaluateDeserializeNestedData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer);
		void EvaluateDeserializeData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer);

		void EvaluateDeserializeIntegralData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer);
		void EvaluateDeserializeFloatingPointData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer);
		void EvaluateDeserializeOtherData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer);

		template<typename T>
		void DeserializeData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
		{
			const auto& dataValue = value.as<T>();
			if (bIsSeqContainer)
			{
				instance.set(data.id(), dataValue);
			}
			else
			{
				SetDataValue(data, instance, dataValue);
			}
		}
		void DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer);

	private:
		std::string m_Path;
	};

	class SceneSerializer : public TypeSerializer
	{
	public:
		SceneSerializer(const std::string& filePath, const Ref<Scene>& scene);

		void Serialize();
		void SerializeRuntime();

	private:
		void SerializeEntity(YAML::Emitter& out, const Entity entity);

	public:
		bool Deserialize();
		bool DeserializeRuntime();

	private:
		void DeserializeEntity(const YAML::Node& entity);

	private:
		Ref<Scene> m_Scene;
	};

}
