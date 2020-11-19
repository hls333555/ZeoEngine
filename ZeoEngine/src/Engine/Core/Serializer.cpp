#include "ZEpch.h"
#include "Engine/Core/Serializer.h"

#include <glm/glm.hpp>
#include <magic_enum.hpp>

#include "Engine/Core/ReflectionHelper.h"
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

	void TypeSerializer::SerializeType(YAML::Emitter& out, entt::meta_any instance)
	{
		for (auto data : instance.type().data())
		{
			// Do not serialize transient data
			auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, data);
			if (bDiscardSerialize) continue;

			const auto dataType = data.type();
			if (dataType.is_sequence_container())
			{
				EvaluateSerializeSequenceContainerData(out, data, instance);
			}
			else if (dataType.is_associative_container())
			{
				EvaluateSerializeAssociativeContainerData(out, data, instance);
			}
			else if (DoesPropExist(PropertyType::NestedClass, dataType))
			{
				EvaluateSerializeNestedData(out, data, instance, false);
			}
			else
			{
				EvaluateSerializeData(out, data, instance, false);
			}
		}
	}

	void TypeSerializer::EvaluateSerializeSequenceContainerData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
	{
		auto& seqView = data.get(instance).as_sequence_container();
		const auto type = seqView.value_type();
		const auto dataName = GetMetaObjectDisplayName(data);
		out << YAML::Key << *dataName << YAML::Value;
		{
			out << YAML::Flow;
			out << YAML::BeginSeq;
			{
				for (auto it = seqView.begin(); it != seqView.end(); ++it)
				{
					auto element = *it;
					bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, type);
					if (bIsNestedClass)
					{
						EvaluateSerializeNestedData(out, data, element, true);
					}
					else
					{
						EvaluateSerializeData(out, data, element, true);
					}
				}
			}
			out << YAML::EndSeq;
		}
	}

	void TypeSerializer::EvaluateSerializeAssociativeContainerData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance)
	{

	}

	void TypeSerializer::EvaluateSerializeNestedData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto dataName = GetMetaObjectDisplayName(data);
		bIsSeqContainer ? out << YAML::BeginSeq : out << YAML::Key << *dataName << YAML::Value << YAML::BeginSeq;
		{
			const auto type = bIsSeqContainer ? instance.type() : data.type();
			auto subInstance = bIsSeqContainer ? instance : data.get(instance);
			for (auto subData : type.data())
			{
				out << YAML::BeginMap;
				{
					bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, subData.type());
					if (bIsNestedClass)
					{
						EvaluateSerializeNestedData(out, subData, subInstance, false);
					}
					else
					{
						EvaluateSerializeData(out, subData, subInstance, false);
					}
				}
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;
	}

	void TypeSerializer::EvaluateSerializeData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (type.is_integral())
		{
			EvaluateSerializeIntegralData(out, data, instance, bIsSeqContainer);
		}
		else if (type.is_floating_point())
		{
			EvaluateSerializeFloatingPointData(out, data, instance, bIsSeqContainer);
		}
		else if (type.is_enum())
		{
			SerializeEnumData(out, data, instance, bIsSeqContainer);
		}
		else
		{
			EvaluateSerializeOtherData(out, data, instance, bIsSeqContainer);
		}
	}

	void TypeSerializer::EvaluateSerializeIntegralData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<bool>(type))
		{
			SerializeData<bool>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<int8_t>(type))
		{
			SerializeData<int8_t>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<int32_t>(type))
		{
			SerializeData<int32_t>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<int64_t>(type))
		{
			SerializeData<int64_t>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<uint8_t>(type))
		{
			SerializeData<uint8_t>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<uint32_t>(type))
		{
			SerializeData<uint32_t>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<uint64_t>(type))
		{
			SerializeData<uint64_t>(out, data, instance, bIsSeqContainer);
		}
	}

	void TypeSerializer::EvaluateSerializeFloatingPointData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<float>(type))
		{
			SerializeData<float>(out, data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<double>(type))
		{
			SerializeData<double>(out, data, instance, bIsSeqContainer);
		}
	}

	void TypeSerializer::EvaluateSerializeOtherData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<std::string>(type))
		{
			SerializeData<std::string>(out, data, instance, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<glm::vec2>(type))
		{
			SerializeData<glm::vec2>(out, data, instance, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<glm::vec3>(type))
		{
			SerializeData<glm::vec3>(out, data, instance, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<glm::vec4>(type))
		{
			SerializeData<glm::vec4>(out, data, instance, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<Ref<Texture2D>>(type))
		{
			SerializeData<Ref<Texture2D>>(out, data, instance, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<Ref<ParticleTemplate>>(type))
		{
			SerializeData<Ref<ParticleTemplate>>(out, data, instance, bIsSeqContainer);
			return;
		}

		auto dataName = GetMetaObjectDisplayName(data);
		ZE_CORE_ASSERT_INFO(false, "Failed to serialize data: '{0}'", *dataName);
	}

	void TypeSerializer::SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, const entt::meta_any instance, bool bIsSeqContainer)
	{
		if (bIsSeqContainer)
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
		YAML::Node data = YAML::LoadFile(m_Path);
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
		for (auto data : instance.type().data())
		{
			auto dataName = GetMetaObjectDisplayName(data);
			const auto& dataValue = value[*dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				const auto dataType = data.type();
				if (dataType.is_sequence_container())
				{
					EvaluateDeserializeSequenceContainerData(data, instance, dataValue);
				}
				else if (dataType.is_associative_container())
				{
					EvaluateDeserializeAssociativeContainerData(data, instance, dataValue);
				}
				else if (DoesPropExist(PropertyType::NestedClass, dataType))
				{
					EvaluateDeserializeNestedData(data, instance, dataValue, false);
				}
				else
				{
					EvaluateDeserializeData(data, instance, dataValue, false);
				}
			}
		}
	}

	static entt::meta_sequence_container::iterator InsertDefaultValueForSeq(entt::meta_data data, entt::meta_sequence_container& seqView)
	{
		// "0" value works for "all" types because we have registered their conversion functions
		auto& [retIt, res] = seqView.insert(seqView.end(), 0);
		if (res)
		{
			return retIt;
		}
		else
		{
			// For special types like user-defined enums, we have to invoke a function instead
			auto defaultValue = CreateTypeDefaultValue(seqView.value_type());
			auto& [retIt, res] = seqView.insert(seqView.end(), defaultValue);
			if (res)
			{
				return retIt;
			}
			else
			{
				auto dataName = GetMetaObjectDisplayName(data);
				ZE_CORE_ASSERT_INFO(false, "Failed to insert with data: '{0}'!", *dataName);
			}
		}
		return {};
	}

	void TypeSerializer::EvaluateDeserializeSequenceContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value)
	{
		const auto type = data.get(instance).as_sequence_container().value_type();
		for (const auto& element : value)
		{
			auto& seqView = data.get(instance).as_sequence_container();
			auto it = InsertDefaultValueForSeq(data, seqView);
			bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, type);
			if (bIsNestedClass)
			{
				EvaluateDeserializeNestedData(data, *it, element, true);
			}
			else
			{
				EvaluateDeserializeData(data, *it, element, true);
			}
		}
	}

	void TypeSerializer::EvaluateDeserializeAssociativeContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value)
	{

	}

	void TypeSerializer::EvaluateDeserializeNestedData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		auto subInstance = bIsSeqContainer ? instance : data.get(instance);
		uint32_t i = 0;
		for (auto subData : type.data())
		{
			auto subDataName = GetMetaObjectDisplayName(subData);
			// Evaluate serialized subdata only
			if (value[i])
			{
				const auto& subValue = value[i][*subDataName];
				// Evaluate serialized subdata only
				if (subValue)
				{
					bool bIsNestedClass = DoesPropExist(PropertyType::NestedClass, subData.type());
					if (bIsNestedClass)
					{
						EvaluateDeserializeNestedData(subData, subInstance, subValue, false);
					}
					else
					{
						EvaluateDeserializeData(subData, subInstance, subValue, false);
					}
				}
			}
			++i;
		}
		// We must set subInstance value back to instance
		SetDataValue(data, instance, subInstance);
	}

	void TypeSerializer::EvaluateDeserializeData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (type.is_integral())
		{
			EvaluateDeserializeIntegralData(data, instance, value, bIsSeqContainer);
		}
		else if (type.is_floating_point())
		{
			EvaluateDeserializeFloatingPointData(data, instance, value, bIsSeqContainer);
		}
		else if (type.is_enum())
		{
			DeserializeEnumData(data, instance, value, bIsSeqContainer);
		}
		else
		{
			EvaluateDeserializeOtherData(data, instance, value, bIsSeqContainer);
		}
	}

	void TypeSerializer::EvaluateDeserializeIntegralData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<bool>(type))
		{
			DeserializeData<bool>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<int8_t>(type))
		{
			DeserializeData<int8_t>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<int32_t>(type))
		{
			DeserializeData<int32_t>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<int64_t>(type))
		{
			DeserializeData<int64_t>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<uint8_t>(type))
		{
			DeserializeData<uint8_t>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<uint32_t>(type))
		{
			DeserializeData<uint32_t>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<uint64_t>(type))
		{
			DeserializeData<uint64_t>(data, instance, value, bIsSeqContainer);
		}
	}

	void TypeSerializer::EvaluateDeserializeFloatingPointData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<float>(type))
		{
			DeserializeData<float>(data, instance, value, bIsSeqContainer);
		}
		else if (IsTypeEqual<double>(type))
		{
			DeserializeData<double>(data, instance, value, bIsSeqContainer);
		}
	}

	void TypeSerializer::EvaluateDeserializeOtherData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<std::string>(type))
		{
			DeserializeData<std::string>(data, instance, value, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<glm::vec2>(type))
		{
			DeserializeData<glm::vec2>(data, instance, value, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<glm::vec3>(type))
		{
			DeserializeData<glm::vec3>(data, instance, value, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<glm::vec4>(type))
		{
			DeserializeData<glm::vec4>(data, instance, value, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<Ref<Texture2D>>(type))
		{
			DeserializeData<Ref<Texture2D>>(data, instance, value, bIsSeqContainer);
			return;
		}
		else if (IsTypeEqual<Ref<ParticleTemplate>>(type))
		{
			DeserializeData<Ref<ParticleTemplate>>(data, instance, value, bIsSeqContainer);
			return;
		}

		auto dataName = GetMetaObjectDisplayName(data);
		ZE_CORE_ASSERT_INFO(false, "Failed to deserialize data: '{0}'", *dataName);
	}

	void TypeSerializer::DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqContainer)
	{
		const auto currentValueName = value.as<std::string>();
		const auto& datas = bIsSeqContainer ? instance.type().data() : data.type().data();
		for (auto enumData : datas)
		{
			auto valueName = GetMetaObjectDisplayName(enumData);
			if (currentValueName == valueName)
			{
				auto newValue = enumData.get({});
				if (bIsSeqContainer)
				{
					SetEnumValueForSeq(instance, newValue);
				}
				else
				{
					SetDataValue(data, instance, newValue);
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
				for (auto typeId : m_Scene->m_Entities[entity])
				{
					out << YAML::BeginMap;
					{
						const auto type = entt::resolve_type(typeId);
						out << YAML::Key << "Component" << YAML::Value << typeId; // TODO: Type ID goes here
						const auto instance = entity.GetTypeById(type.type_id());
						SerializeType(out, instance);
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
				auto typeId = component["Component"].as<uint32_t>();
				// TODO: NativeScriptComponent deserialization
				if (typeId == entt::type_info<NativeScriptComponent>().id()) continue;

				entt::meta_any instance = deserializedEntity.GetOrAddTypeById(typeId);
				// Instance may be null as typeId is invalid
				if (instance)
				{
					DeserializeType(instance, component);
				}
			}
		}
	}

}
