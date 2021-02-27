#include "ZEpch.h"
#include "Engine/Core/Serializer.h"

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

	template<>
	struct convert<Ref<ParticleTemplate>>
	{
		static Node encode(const Ref<ParticleTemplate>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, Ref<ParticleTemplate>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = ParticleLibrary::Get().GetOrLoad(path);
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

	YAML::Emitter& operator<<(YAML::Emitter& out, const Ref<ParticleTemplate>& pTemplate)
	{
		out << (pTemplate ? pTemplate->GetPath() : "");
		return out;
	}

	//////////////////////////////////////////////////////////////////////////
	// TypeSerializer ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	TypeSerializer::TypeSerializer(const std::string& filePath)
		: m_Path(filePath)
	{
	}

	void TypeSerializer::Serialize(entt::meta_any instance, AssetType assetType)
	{
		Serialize_t(assetType, [&](YAML::Emitter& out)
		{
			SerializeType(out, instance);
		});
	}

	void TypeSerializer::SerializeType(YAML::Emitter& out, entt::meta_any& instance)
	{
		for (const auto data : instance.type().data())
		{
			// Do not serialize transient data
			auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, data);
			if (bDiscardSerialize) continue;

			EvaluateSerializeData(out, data, instance, false);
		}
	}

	void TypeSerializer::EvaluateSerializeData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		switch (EvaluateMetaType(type))
		{
		case BasicMetaType::STRUCT:
			EvaluateSerializeStructData(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::SEQCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return;
			}
			EvaluateSerializeSequenceContainerData(out, data, instance);
			break;
		case BasicMetaType::ASSCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return;
			}
			EvaluateSerializeAssociativeContainerData(out, data, instance);
			break;
		case BasicMetaType::BOOL:
			SerializeData<bool>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::I8:
			SerializeData<int8_t>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::I32:
			SerializeData<int32_t>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::I64:
			SerializeData<int64_t>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::UI8:
			SerializeData<uint8_t>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::UI32:
			SerializeData<uint32_t>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::UI64:
			SerializeData<uint64_t>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::FLOAT:
			SerializeData<float>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::DOUBLE:
			SerializeData<double>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::ENUM:
			SerializeEnumData(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::STRING:
			SerializeData<std::string>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::VEC2:
			SerializeData<glm::vec2>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::VEC3:
			SerializeData<glm::vec3>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::VEC4:
			SerializeData<glm::vec4>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::TEXTURE:
			SerializeData<Ref<Texture2D>>(out, data, instance, bIsSeqElement);
			break;
		case BasicMetaType::PARTICLE:
			SerializeData<Ref<ParticleTemplate>>(out, data, instance, bIsSeqElement);
			break;
		default:
			auto dataName = GetMetaObjectDisplayName(data);
			ZE_CORE_ASSERT(false, "Failed to serialize data: '{0}'", *dataName);
			break;
		}
	}

	void TypeSerializer::EvaluateSerializeSequenceContainerData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance)
	{
		auto seqView = data.get(instance).as_sequence_container();
		const auto dataName = GetMetaObjectDisplayName(data);
		out << YAML::Key << *dataName << YAML::Value;
		{
			out << YAML::Flow;
			out << YAML::BeginSeq;
			{
				for (auto it = seqView.begin(); it != seqView.end(); ++it)
				{
					auto elementInstance = *it;
					EvaluateSerializeData(out, data, elementInstance, true);
				}
			}
			out << YAML::EndSeq;
		}
	}

	void TypeSerializer::EvaluateSerializeAssociativeContainerData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance)
	{

	}

	void TypeSerializer::EvaluateSerializeStructData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		const auto dataName = GetMetaObjectDisplayName(data);
		bIsSeqElement ? out << YAML::BeginSeq : out << YAML::Key << *dataName << YAML::Value << YAML::BeginSeq;
		{
			const auto structType = bIsSeqElement ? instance.type() : data.type();
			auto structInstance = bIsSeqElement ? instance : data.get(instance);
			for (const auto subData : structType.data())
			{
				out << YAML::BeginMap;
				{
					EvaluateSerializeData(out, subData, structInstance, false);
				}
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;
	}

	void TypeSerializer::SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		if (bIsSeqElement)
		{
			const char* enumValueName = GetEnumDisplayName(instance);
			out << enumValueName;
		}
		else
		{
			const auto dataName = GetMetaObjectDisplayName(data);
			const auto enumValue = data.get(instance);
			const char* enumValueName = GetEnumDisplayName(enumValue);
			out << YAML::Key << *dataName << YAML::Value << enumValueName;
		}
	}

	bool TypeSerializer::Deserialize(entt::meta_any instance, AssetType assetType)
	{
		auto data = PreDeserialize(assetType);
		if (!data) return false;

		DeserializeType(instance, *data);
		return true;
	}

	std::optional<YAML::Node> TypeSerializer::PreDeserialize(AssetType assetType)
	{
		auto data = YAML::LoadFile(m_Path);
		auto assetTypeName = magic_enum::enum_name(assetType).data();
		if (!data[assetTypeName])
		{
			const std::string fileName = GetFileNameFromPath(m_Path);
			ZE_CORE_ERROR("Failed to load {0}. Unknown {1} format!", fileName, assetTypeName);
			return {};
		}

		std::string assetName = data[assetTypeName].as<std::string>();
		ZE_CORE_TRACE("Deserializing {0} '{1}'", assetTypeName, assetName);
		return data;
	}

	void TypeSerializer::DeserializeType(entt::meta_any& instance, const YAML::Node& value)
	{
		for (const auto data : instance.type().data())
		{
			auto dataName = GetMetaObjectDisplayName(data);
			const auto& dataValue = value[*dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(data, instance, dataValue, false);
			}
		}
	}

	void TypeSerializer::EvaluateDeserializeData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		switch (EvaluateMetaType(type))
		{
		case BasicMetaType::STRUCT:
			EvaluateDeserializeStructData(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::SEQCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return;
			}
			EvaluateDeserializeSequenceContainerData(data, instance, value);
			break;
		case BasicMetaType::ASSCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return;
			}
			EvaluateDeserializeAssociativeContainerData(data, instance, value);
			break;
		case BasicMetaType::BOOL:
			DeserializeData<bool>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::I8:
			DeserializeData<int8_t>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::I32:
			DeserializeData<int32_t>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::I64:
			DeserializeData<int64_t>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::UI8:
			DeserializeData<uint8_t>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::UI32:
			DeserializeData<uint32_t>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::UI64:
			DeserializeData<uint64_t>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::FLOAT:
			DeserializeData<float>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::DOUBLE:
			DeserializeData<double>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::ENUM:
			DeserializeEnumData(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::STRING:
			DeserializeData<std::string>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::VEC2:
			DeserializeData<glm::vec2>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::VEC3:
			DeserializeData<glm::vec3>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::VEC4:
			DeserializeData<glm::vec4>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::TEXTURE:
			DeserializeData<Ref<Texture2D>>(data, instance, value, bIsSeqElement);
			break;
		case BasicMetaType::PARTICLE:
			DeserializeData<Ref<ParticleTemplate>>(data, instance, value, bIsSeqElement);
			break;
		default:
			auto dataName = GetMetaObjectDisplayName(data);
			ZE_CORE_ASSERT(false, "Failed to deserialize data: '{0}'", *dataName);
			break;
		}
	}

	static entt::meta_sequence_container::iterator InsertDefaultValueForSeq(const entt::meta_data data, entt::meta_sequence_container& seqView)
	{
		auto [retIt, res] = seqView.insert(seqView.end(), seqView.value_type().construct());
		if (res)
		{
			return retIt;
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(data);
			ZE_CORE_ASSERT(false, "Failed to insert with data: '{0}'! Please check if its type is properly registered.", *dataName);
		}

		return {};
	}

	void TypeSerializer::EvaluateDeserializeSequenceContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value)
	{
		for (const auto& elementValue : value)
		{
			auto seqView = data.get(instance).as_sequence_container();
			auto it = InsertDefaultValueForSeq(data, seqView);
			auto elementInstance = *it;
			EvaluateDeserializeData(data, elementInstance, elementValue, true);
		}
	}

	void TypeSerializer::EvaluateDeserializeAssociativeContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value)
	{

	}

	void TypeSerializer::EvaluateDeserializeStructData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		auto structInstance = bIsSeqElement ? instance : data.get(instance);
		uint32_t i = 0;
		for (const auto subData : type.data())
		{
			auto subDataName = GetMetaObjectDisplayName(subData);
			// Evaluate serialized subdata only
			if (value[i])
			{
				const auto& subValue = value[i][*subDataName];
				// Evaluate serialized subdata only
				if (subValue)
				{
					EvaluateDeserializeData(subData, structInstance, subValue, false);
				}
			}
			++i;
		}
	}

	void TypeSerializer::DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
	{
		const auto currentValueName = value.as<std::string>();
		const auto& datas = bIsSeqElement ? instance.type().data() : data.type().data();
		for (const auto enumData : datas)
		{
			auto valueName = GetMetaObjectDisplayName(enumData);
			if (currentValueName == valueName)
			{
				auto newValue = enumData.get({});
				if (bIsSeqElement)
				{
					Reflection::SetEnumValueForSeq(instance, newValue);
				}
				else
				{
					data.set(instance, newValue);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneSerializer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	SceneSerializer::SceneSerializer(const std::string& filePath, const Ref<Scene>& scene)
		: TypeSerializer(filePath)
		, m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize()
	{
		Serialize_t(AssetType::Scene, [&](YAML::Emitter& out)
		{
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
		});
	}

	void SceneSerializer::SerializeRuntime()
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
				// Do not call entt::registry::visit() as the order is reversed
				for (const auto compId : entity.GetOrderedComponentIds())
				{
					out << YAML::BeginMap;
					{
						out << YAML::Key << "Component" << YAML::Value << compId; // TODO: Component ID goes here
						auto compInstance = entity.GetComponentById(compId);
						SerializeType(out, compInstance);
					}
					out << YAML::EndMap;
				}
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;
	}

	bool SceneSerializer::Deserialize()
	{
		auto data = PreDeserialize(AssetType::Scene);
		if (!data) return false;

		auto entities = (*data)["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				DeserializeEntity(entity);
			}
		}
		return true;
	}

	bool SceneSerializer::DeserializeRuntime()
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
		return false;
	}

	void SceneSerializer::DeserializeEntity(const YAML::Node& entity)
	{
		uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO: UUID
		// Some entities including preview camera are created by default on scene creation
		Entity deserializedEntity{ static_cast<entt::entity>(uuid), m_Scene.get() };
		if (!deserializedEntity.IsValid())
		{
			// Create a default entity if not exists
			deserializedEntity = m_Scene->CreateEntity();
		}

		auto components = entity["Components"];
		if (components)
		{
			for (auto component : components)
			{
				auto compId = component["Component"].as<uint32_t>();
				// TODO: NativeScriptComponent deserialization
				if (compId == entt::type_hash<NativeScriptComponent>::value()) continue;

				entt::meta_any compInstance = deserializedEntity.GetOrAddComponentById(compId);
				// Instance may be null as compId is invalid
				if (compInstance)
				{
					DeserializeType(compInstance, component);
				}
			}
		}
	}

}
