#include "ZEpch.h"
#include "Engine/Core/SceneSerializer.h"

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/Components.h"

namespace YAML {

	using namespace ZeoEngine;

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Ref<Texture2D>>
	{
		static Node encode(const Ref<Texture2D>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, Ref<Texture2D>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = Texture2DLibrary::Get().GetOrLoad(path);
			return true;
		}
	};

}

namespace ZeoEngine {

	YAML::Emitter& operator<<(YAML::Emitter& out, const Entity& e)
	{
		out << e.GetEntityId();
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Ref<Texture2D>& texture)
	{
		out << (texture ? texture->GetPath() : "");
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// Serialization /////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void SceneSerializer::Serialize(const std::string& filePath)
	{
		YAML::Emitter out;

		// TODO: Scene name
		const char* sceneName = "Untitled";
		ZE_CORE_TRACE("Serializing scene '{0}'", sceneName);

		out << YAML::BeginMap;
		{
			out << YAML::Key << "Scene" << YAML::Value << sceneName;
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			{
				m_Scene->m_Registry.view<CoreComponent>().each([&](auto entityId, auto& cc)
				{
					Entity entity = { entityId, m_Scene.get() };
					if (!entity) return;

					SerializeEntity(out, entity);
				});
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filePath)
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, const Entity entity)
	{
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Entity" << YAML::Value << entity; // TODO: Entity ID goes here
			out << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
			{
				m_Scene->m_Registry.visit(entity, [&](const auto typeId)
				{
					out << YAML::BeginMap;
					{
						const auto type = entt::resolve_type(typeId);
						out << YAML::Key << "Component" << YAML::Value << typeId; // TODO: Type ID goes here
						SerializeType(out, type, entity);
					}
					out << YAML::EndMap;
				});
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeType(YAML::Emitter& out, const entt::meta_type type, const Entity entity)
	{
		const auto instance = GetTypeInstance(type, m_Scene->m_Registry, entity);
		type.data([&](entt::meta_data data)
		{
			// Do not serialize transient data
			auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, data);
			if (bDiscardSerialize) return;

			if (data.type().is_integral())
			{
				SerializeIntegralData(out, data, instance);
			}
			else if (data.type().is_floating_point())
			{
				SerializeFloatingPointData(out, data, instance);
			}
			else if (data.type().is_enum())
			{
				SerializeEnumData(out, data, instance);
			}
			else
			{
				SerializeOtherData(out, data, instance);
			}
		});
	}

	void SceneSerializer::SerializeIntegralData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
	{
		if (IsTypeEqual<bool>(data.type()))
		{
			SerializeData<bool>(out, data, instance);
		}
		else if (IsTypeEqual<int8_t>(data.type()))
		{
			SerializeData<int8_t>(out, data, instance);
		}
		else if (IsTypeEqual<int32_t>(data.type()))
		{
			SerializeData<int32_t>(out, data, instance);
		}
		else if (IsTypeEqual<int64_t>(data.type()))
		{
			SerializeData<int64_t>(out, data, instance);
		}
		else if (IsTypeEqual<uint8_t>(data.type()))
		{
			SerializeData<uint8_t>(out, data, instance);
		}
		else if (IsTypeEqual<uint32_t>(data.type()))
		{
			SerializeData<uint32_t>(out, data, instance);
		}
		else if (IsTypeEqual<uint64_t>(data.type()))
		{
			SerializeData<uint64_t>(out, data, instance);
		}
	}

	void SceneSerializer::SerializeFloatingPointData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
	{
		if (IsTypeEqual<float>(data.type()))
		{
			SerializeData<float>(out, data, instance);
		}
		else if (IsTypeEqual<double>(data.type()))
		{
			SerializeData<double>(out, data, instance);
		}
	}

	void SceneSerializer::SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
	{
		const auto dataName = GetMetaObjectDisplayName(data);
		const char* enumValueName = nullptr;
		const auto enumValue = data.get(instance);
		data.type().data([enumValue, &enumValueName](entt::meta_data enumData)
		{
			if (enumValue == enumData.get({}))
			{
				auto valueName = GetMetaObjectDisplayName(enumData);
				enumValueName = *valueName;
			}
		});
		out << YAML::Key << *dataName << YAML::Value << enumValueName;
	}

	void SceneSerializer::SerializeOtherData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
	{
		if (IsTypeEqual<std::string>(data.type()))
		{
			SerializeData<std::string>(out, data, instance);
			return;
		}
		else if (IsTypeEqual<glm::vec2>(data.type()))
		{
			SerializeData<glm::vec2>(out, data, instance);
			return;
		}
		else if (IsTypeEqual<glm::vec3>(data.type()))
		{
			SerializeData<glm::vec3>(out, data, instance);
			return;
		}
		else if (IsTypeEqual<glm::vec4>(data.type()))
		{
			SerializeData<glm::vec4>(out, data, instance);
			return;
		}
		else if (IsTypeEqual<Ref<Texture2D>>(data.type()))
		{
			SerializeData<Ref<Texture2D>>(out, data, instance);
			return;
		}

		auto dataName = GetMetaObjectDisplayName(data);
		ZE_CORE_ASSERT_INFO(false, "Failed to serialize data: '{0}'", *dataName);
	}

	//////////////////////////////////////////////////////////////////////////
	// Deserialization ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool SceneSerializer::Deserialize(const std::string& filePath)
	{
		std::ifstream stream(filePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
		{
			ZE_CORE_ERROR("Failed to load scene. Unknown scene format!");
			return false;
		}

		std::string sceneName = data["Scene"].as<std::string>();
		ZE_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				//uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO: UUID

				// Create a default entity
				Entity deserializedEntity = m_Scene->CreateEntity();

				auto components = entity["Components"];
				if (components)
				{
					for (auto component : components)
					{
						auto typeId = component["Component"].as<uint32_t>();
						// TODO: NativeScriptComponent deserialization
						if (typeId == entt::type_info<NativeScriptComponent>().id()) continue;

						auto type = entt::resolve_type(typeId);
						auto bIsInherentType = DoesPropExist(PropertyType::InherentType, type);
						entt::meta_any instance;
						if (bIsInherentType)
						{
							// Get inherent type from that entity as it has already been added on entity creation
							instance = GetTypeInstance(type, m_Scene->m_Registry, deserializedEntity);
						}
						else
						{
							// Add type to that entity
							instance = AddTypeById(typeId, m_Scene->m_Registry, deserializedEntity);
						}
						
						// Iterate all datas and deserialize values
						type.data([this, instance, component](entt::meta_data data)
						{
							auto dataName = GetMetaObjectDisplayName(data);
							const auto& value = component[*dataName];
							if (value)
							{
								// Evaluate serialized data only
								EvaluateData(data, instance, value);
							}
						});
					}
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filePath)
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
		return false;
	}

	void SceneSerializer::EvaluateData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value)
	{
		if (data.type().is_integral())
		{
			DeserializeIntegralData(data, instance, value);
		}
		else if (data.type().is_floating_point())
		{
			DeserializeFloatingPointData(data, instance, value);
		}
		else if (data.type().is_enum())
		{
			DeserializeEnumData(data, instance, value);
		}
		else
		{
			DeserializeOtherData(data, instance, value);
		}
	}

	void SceneSerializer::DeserializeIntegralData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value)
	{
		if (IsTypeEqual<bool>(data.type()))
		{
			DeserializeData<bool>(data, instance, value);
		}
		else if (IsTypeEqual<int8_t>(data.type()))
		{
			DeserializeData<int8_t>(data, instance, value);
		}
		else if (IsTypeEqual<int32_t>(data.type()))
		{
			DeserializeData<int32_t>(data, instance, value);
		}
		else if (IsTypeEqual<int64_t>(data.type()))
		{
			DeserializeData<int64_t>(data, instance, value);
		}
		else if (IsTypeEqual<uint8_t>(data.type()))
		{
			DeserializeData<uint8_t>(data, instance, value);
		}
		else if (IsTypeEqual<uint32_t>(data.type()))
		{
			DeserializeData<uint32_t>(data, instance, value);
		}
		else if (IsTypeEqual<uint64_t>(data.type()))
		{
			DeserializeData<uint64_t>(data, instance, value);
		}
	}

	void SceneSerializer::DeserializeFloatingPointData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value)
	{
		if (IsTypeEqual<float>(data.type()))
		{
			DeserializeData<float>(data, instance, value);
		}
		else if (IsTypeEqual<double>(data.type()))
		{
			DeserializeData<double>(data, instance, value);
		}
	}

	void SceneSerializer::DeserializeEnumData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value)
	{
		auto currentValueName = value.as<std::string>();
		data.type().data([&](entt::meta_data enumData)
		{
			auto valueName = GetMetaObjectDisplayName(enumData);
			if (currentValueName == valueName)
			{
				auto newValue = enumData.get({});
				SetDataValue(data, instance, newValue);
			}
		});
	}

	void SceneSerializer::DeserializeOtherData(entt::meta_data data, entt::meta_any instance, const YAML::Node& value)
	{
		if (IsTypeEqual<std::string>(data.type()))
		{
			DeserializeData<std::string>(data, instance, value);
			return;
		}
		else if (IsTypeEqual<glm::vec2>(data.type()))
		{
			DeserializeData<glm::vec2>(data, instance, value);
			return;
		}
		else if (IsTypeEqual<glm::vec3>(data.type()))
		{
			DeserializeData<glm::vec3>(data, instance, value);
			return;
		}
		else if (IsTypeEqual<glm::vec4>(data.type()))
		{
			DeserializeData<glm::vec4>(data, instance, value);
			return;
		}
		else if (IsTypeEqual<Ref<Texture2D>>(data.type()))
		{
			DeserializeData<Ref<Texture2D>>(data, instance, value);
			return;
		}

		auto dataName = GetMetaObjectDisplayName(data);
		ZE_CORE_ASSERT_INFO(false, "Failed to deserialize data: '{0}'", *dataName);
	}

}
