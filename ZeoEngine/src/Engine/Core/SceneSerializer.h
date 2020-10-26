#pragma once

#include "Engine/GameFramework/Entity.h"

#include <yaml-cpp/yaml.h>

// TODO:
#include "../../ZeoEditor/src/Reflection/ReflectionHelper.h"

namespace ZeoEngine {

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filePath);
		void SerializeRuntime(const std::string& filePath);

	private:
		void SerializeEntity(YAML::Emitter& out, const Entity entity);
		void SerializeType(YAML::Emitter& out, entt::meta_type type, const Entity entity);

		void SerializeIntegralData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void SerializeFloatingPointData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);
		void SerializeOtherData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance);

		template<typename T>
		void SerializeData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
		{
			const auto dataName = GetMetaObjectDisplayName(data);
			const auto dataValue = GetDataValue<T>(data, instance);
			out << YAML::Key << *dataName << YAML::Value << dataValue;
		}

	public:
		bool Deserialize(const std::string& filePath);
		bool DeserializeRuntime(const std::string& filePath);

	private:
		void EvaluateData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);
		void DeserializeIntegralData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);
		void DeserializeFloatingPointData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);
		void DeserializeEnumData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);
		void DeserializeOtherData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value);

		template<typename T>
		void DeserializeData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value)
		{
			SetDataValue(data, instance, value.as<T>());
		}

	private:
		Ref<Scene> m_Scene;
	};

}
