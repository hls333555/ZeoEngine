#pragma once

#include "Engine/GameFramework/Entity.h"

#include <yaml-cpp/yaml.h>

#include "Engine/Core/ReflectionHelper.h"

namespace ZeoEngine {

	enum class SerializerType
	{
		Scene,
		ParticleSystem,
	};

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene, SerializerType type);

		void Serialize(const std::string& filePath);
		void SerializeRuntime(const std::string& filePath);

	private:
		void SerializeEntity(YAML::Emitter& out, const Entity entity);
		void SerializeType(YAML::Emitter& out, entt::meta_type type, const Entity entity);

		void EvaluateSerializeSequenceContainerData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void EvaluateSerializeAssociativeContainerData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void EvaluateSerializeData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);

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
		bool Deserialize(const std::string& filePath);
		bool DeserializeRuntime(const std::string& filePath);

	private:
		void DeserializeEntity(const YAML::Node& entity);

		void EvaluateDeserializeSequenceContainerData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);
		void EvaluateDeserializeAssociativeContainerData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);
		void EvaluateDeserializeData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);

		void EvaluateDeserializeIntegralData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value, bool bIsSeqContainer);
		void EvaluateDeserializeFloatingPointData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value, bool bIsSeqContainer);
		void EvaluateDeserializeOtherData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value, bool bIsSeqContainer);

		template<typename T>
		void DeserializeData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value, bool bIsSeqContainer)
		{
			if (bIsSeqContainer)
			{
				auto& seqView = data.get(instance).as_sequence_container();
				seqView.insert(seqView.end(), value.as<T>());
			}
			else
			{
				SetDataValue(data, instance, value.as<T>());
			}
		}
		void DeserializeEnumData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value, bool bIsSeqContainer);

	private:
		Ref<Scene> m_Scene;
		SerializerType m_SerializerType;
	};

}
