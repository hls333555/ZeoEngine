#include "ZEpch.h"
#include "Engine/Core/Serializer.h"

#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Asset/AssetLibrary.h"

namespace YAML {

#define DEFINE_ASSET_CONVERTOR(assetClass)									\
	template<>																\
	struct convert<Ref<assetClass>>											\
	{																		\
		static Node encode(const Ref<assetClass>& rhs)						\
		{																	\
			Node node;														\
			node = rhs ? rhs->GetHandle() : 0;								\
			return node;													\
		}																	\
		static bool decode(const Node& node, Ref<assetClass>& rhs)			\
		{																	\
			const auto handle = node.as<AssetHandle>();						\
			if (!handle) return true;										\
			rhs = AssetLibrary::LoadAsset<assetClass>(handle);				\
			return true;													\
		}																	\
	};

	DEFINE_ASSET_CONVERTOR(Texture2D);
	DEFINE_ASSET_CONVERTOR(ParticleTemplate);
	DEFINE_ASSET_CONVERTOR(Mesh);
	DEFINE_ASSET_CONVERTOR(Material);
	DEFINE_ASSET_CONVERTOR(Shader);

}

namespace ZeoEngine {

	//////////////////////////////////////////////////////////////////////////
	// ComponentSerializer ///////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ComponentSerializer::Serialize(YAML::Node& node, entt::meta_any instance)
	{
		if (!instance) return;

		PreprocessDatas(instance);

		for (const auto data : m_PreprocessedDatas)
		{
			// Do not serialize transient data
			auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, data);
			if (bDiscardSerialize) continue;

			EvaluateSerializeData(node, data, instance, false);
		}
	}

	void ComponentSerializer::PreprocessDatas(entt::meta_any& instance)
	{
		m_PreprocessedDatas.clear();
		for (const auto data : instance.type().data())
		{
			m_PreprocessedDatas.push_front(data);
		}
	}

	void ComponentSerializer::EvaluateSerializeData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		switch (EvaluateMetaType(type))
		{
			case BasicMetaType::STRUCT:
				EvaluateSerializeStructData(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::SEQCON:
				if (bIsSeqElement)
				{
					ZE_CORE_ERROR("Container nesting is not supported!");
					return;
				}
				EvaluateSerializeSequenceContainerData(node, data, instance);
				break;
			case BasicMetaType::ASSCON:
				if (bIsSeqElement)
				{
					ZE_CORE_ERROR("Container nesting is not supported!");
					return;
				}
				EvaluateSerializeAssociativeContainerData(node, data, instance);
				break;
			case BasicMetaType::BOOL:
				SerializeData<bool>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::I8:
				SerializeData<I8>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::I32:
				SerializeData<I32>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::I64:
				SerializeData<I64>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::UI8:
				SerializeData<U8>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::UI32:
				SerializeData<U32>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::UI64:
				SerializeData<U64>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::FLOAT:
				SerializeData<float>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::DOUBLE:
				SerializeData<double>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::ENUM:
				SerializeEnumData(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::STRING:
				SerializeData<std::string>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::VEC2:
				SerializeData<Vec2>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::VEC3:
				SerializeData<Vec3>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::VEC4:
				SerializeData<Vec4>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::TEXTURE:
				SerializeData<Ref<Texture2D>>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::PARTICLE:
				SerializeData<Ref<ParticleTemplate>>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::MESH:
				SerializeData<Ref<Mesh>>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::MATERIAL:
				SerializeData<Ref<Material>>(node, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::SHADER:
				SerializeData<Ref<Shader>>(node, data, instance, bIsSeqElement);
				break;
			default:
				const auto dataName = GetMetaObjectDisplayName(data);
				ZE_CORE_ASSERT(false, "Failed to serialize data: '{0}'", *dataName);
				break;
		}
	}

	void ComponentSerializer::EvaluateSerializeSequenceContainerData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance)
	{
		auto seqView = data.get(instance).as_sequence_container();
		const auto dataName = GetMetaObjectDisplayName(data);
		YAML::Node seqNode;
		seqNode.SetStyle(YAML::EmitterStyle::Flow);
		for (auto it = seqView.begin(); it != seqView.end(); ++it)
		{
			auto elementInstance = *it;
			EvaluateSerializeData(seqNode, data, elementInstance, true);
		}
		node[*dataName] = seqNode;
	}

	void ComponentSerializer::EvaluateSerializeAssociativeContainerData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance)
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
	}

	void ComponentSerializer::EvaluateSerializeStructData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		YAML::Node subNode;
		const auto structType = bIsSeqElement ? instance.type() : data.type();
		auto structInstance = bIsSeqElement ? instance.as_ref() : data.get(instance); // NOTE: We must call as_ref() to return reference here or it will return a copy since entt 3.7.0
		for (const auto subData : structType.data())
		{
			EvaluateSerializeData(subNode, subData, structInstance, false);
		}
		if (bIsSeqElement)
		{
			node.push_back(subNode);
		}
		else
		{
			const auto dataName = GetMetaObjectDisplayName(data);
			node[*dataName] = subNode;
		}
	}

	void ComponentSerializer::SerializeEnumData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		if (bIsSeqElement)
		{
			const char* enumValueName = GetEnumDisplayName(instance);
			node.push_back(enumValueName);
		}
		else
		{
			const auto dataName = GetMetaObjectDisplayName(data);
			const auto enumValue = data.get(instance);
			const char* enumValueName = GetEnumDisplayName(enumValue);
			node[*dataName] = enumValueName;
		}
	}

	void ComponentSerializer::Deserialize(const YAML::Node& node, entt::meta_any instance)
	{
		if (!instance) return;

		PreprocessDatas(instance);

		for (const auto data : m_PreprocessedDatas)
		{
			auto dataName = GetMetaObjectDisplayName(data);
			const auto& dataValue = node[*dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(data, instance, dataValue, false);
				const auto* comp = instance.try_cast<IComponent>();
				if (comp->ComponentHelper)
				{
					comp->ComponentHelper->PostDataDeserialize(data.id());
				}
			}
		}
	}

	void ComponentSerializer::EvaluateDeserializeData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		switch (EvaluateMetaType(type))
		{
			case BasicMetaType::STRUCT:
				EvaluateDeserializeStructData(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::SEQCON:
				if (bIsSeqElement)
				{
					ZE_CORE_ERROR("Container nesting is not supported!");
					return;
				}
				EvaluateDeserializeSequenceContainerData(data, instance, node);
				break;
			case BasicMetaType::ASSCON:
				if (bIsSeqElement)
				{
					ZE_CORE_ERROR("Container nesting is not supported!");
					return;
				}
				EvaluateDeserializeAssociativeContainerData(data, instance, node);
				break;
			case BasicMetaType::BOOL:
				DeserializeData<bool>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::I8:
				DeserializeData<I8>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::I32:
				DeserializeData<I32>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::I64:
				DeserializeData<I64>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::UI8:
				DeserializeData<U8>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::UI32:
				DeserializeData<U32>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::UI64:
				DeserializeData<U64>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::FLOAT:
				DeserializeData<float>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::DOUBLE:
				DeserializeData<double>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::ENUM:
				DeserializeEnumData(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::STRING:
				DeserializeData<std::string>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::VEC2:
				DeserializeData<Vec2>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::VEC3:
				DeserializeData<Vec3>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::VEC4:
				DeserializeData<Vec4>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::TEXTURE:
				DeserializeData<Ref<Texture2D>>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::PARTICLE:
				DeserializeData<Ref<ParticleTemplate>>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::MESH:
				DeserializeData<Ref<Mesh>>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::MATERIAL:
				DeserializeData<Ref<Material>>(data, instance, node, bIsSeqElement);
				break;
			case BasicMetaType::SHADER:
				DeserializeData<Ref<Shader>>(data, instance, node, bIsSeqElement);
				break;
			default:
				auto dataName = GetMetaObjectDisplayName(data);
				ZE_CORE_ASSERT(false, "Failed to deserialize data: '{0}'", *dataName);
				break;
		}
	}

	static entt::meta_sequence_container::iterator InsertDefaultValueForSeq(const entt::meta_data data, entt::meta_sequence_container& seqView)
	{
		auto retIt = seqView.insert(seqView.end(), seqView.value_type().construct());
		if (!retIt)
		{
			auto dataName = GetMetaObjectDisplayName(data);
			ZE_CORE_ASSERT(false, "Failed to insert with data: '{0}'! Please check if its type is properly registered.", *dataName);
		}
		return retIt;
	}

	void ComponentSerializer::EvaluateDeserializeSequenceContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& node)
	{
		auto seqView = data.get(instance).as_sequence_container();
		// Clear elements first
		seqView.clear();
		for (const auto& elementValue : node)
		{
			auto it = InsertDefaultValueForSeq(data, seqView);
			auto elementInstance = *it;
			EvaluateDeserializeData(data, elementInstance, elementValue, true);
		}
	}

	void ComponentSerializer::EvaluateDeserializeAssociativeContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& node)
	{

	}

	void ComponentSerializer::EvaluateDeserializeStructData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		auto structInstance = bIsSeqElement ? instance.as_ref() : data.get(instance); // NOTE: We must call as_ref() to return reference here or it will return a copy since entt 3.7.0
		U32 i = 0;
		for (const auto subData : type.data())
		{
			auto subDataName = GetMetaObjectDisplayName(subData);
			// Evaluate serialized subdata only
			if (node[i])
			{
				const auto& subValue = node[i][*subDataName];
				// Evaluate serialized subdata only
				if (subValue)
				{
					EvaluateDeserializeData(subData, structInstance, subValue, false);
				}
			}
			++i;
		}
	}

	void ComponentSerializer::DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement)
	{
		const auto currentValueName = node.as<std::string>();
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
	// MaterialSerializer ////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialSerializer::Serialize(YAML::Node& node, const Ref<Material>& material)
	{
		if (!material) return;

		for (const auto& uniform : material->GetDynamicBindableUniforms())
		{
			EvaluateSerializeData(node, uniform);
		}
		for (const auto& uniform : material->GetDynamicUniforms())
		{
			EvaluateSerializeData(node, uniform);
		}
	}

	void MaterialSerializer::EvaluateSerializeData(YAML::Node& node, const Ref<DynamicUniformDataBase>& uniform)
	{
		switch (uniform->GetDataType())
		{
			case ShaderReflectionType::Bool:
				SerializeData<bool>(node, uniform);
				break;
			case ShaderReflectionType::Int:
				SerializeData<I32>(node, uniform);
				break;
			case ShaderReflectionType::Float:
				SerializeData<float>(node, uniform);
				break;
			case ShaderReflectionType::Vec2:
				SerializeData<Vec2>(node, uniform);
				break;
			case ShaderReflectionType::Vec3:
				SerializeData<Vec3>(node, uniform);
				break;
			case ShaderReflectionType::Vec4:
				SerializeData<Vec4>(node, uniform);
				break;
			case ShaderReflectionType::Texture2D:
				SerializeData<Ref<Texture2D>>(node, uniform);
				break;
			default:
				break;
		}
	}

	void MaterialSerializer::Deserialize(const YAML::Node& node, const Ref<Material>& material)
	{
		if (!material) return;

		for (const auto& uniform : material->GetDynamicBindableUniforms())
		{
			const auto& dataName = uniform->Name;
			const auto& dataValue = node[dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(dataValue, uniform);
			}
		}
		for (const auto& uniform : material->GetDynamicUniforms())
		{
			const auto& dataName = uniform->Name;
			const auto& dataValue = node[dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(dataValue, uniform);
			}
		}
	}

	void MaterialSerializer::EvaluateDeserializeData(const YAML::Node& node, const Ref<DynamicUniformDataBase>& uniform)
	{
		switch (uniform->GetDataType())
		{
			case ShaderReflectionType::Bool:
				DeserializeData<bool>(node, uniform);
				break;
			case ShaderReflectionType::Int:
				DeserializeData<I32>(node, uniform);
				break;
			case ShaderReflectionType::Float:
				DeserializeData<float>(node, uniform);
				break;
			case ShaderReflectionType::Vec2:
				DeserializeData<Vec2>(node, uniform);
				break;
			case ShaderReflectionType::Vec3:
				DeserializeData<Vec3>(node, uniform);
				break;
			case ShaderReflectionType::Vec4:
				DeserializeData<Vec4>(node, uniform);
				break;
			case ShaderReflectionType::Texture2D:
				DeserializeData<Ref<Texture2D>>(node, uniform);
				break;
			default:
				break;
		}
	}

	void SceneSerializer::Serialize(YAML::Node& node, const Ref<Scene>& scene)
	{
		if (!scene) return;

		scene->m_Registry.view<CoreComponent>().each([&](auto entityID, auto& cc)
		{
			const Entity entity = { entityID, scene };
			if (!entity) return;

			YAML::Node entityNode;
			SerializeEntity(entityNode, entity);
			node["Entities"].push_back(entityNode);
		});
	}

	void SceneSerializer::SerializeRuntime()
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
	}

	void SceneSerializer::Deserialize(const YAML::Node& node, const Ref<Scene>& scene)
	{
		if (auto entities = node["Entities"])
		{
			for (auto entity : entities)
			{
				DeserializeEntity(entity, scene);
			}
		}
	}

	void SceneSerializer::DeserializeRuntime()
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
	}

	void SceneSerializer::SerializeEntity(YAML::Node& node, const Entity entity)
	{
		node["Entity"] = entity.GetUUID();
		// Do not call entt::registry::visit() as the order is reversed
		for (const auto compID : entity.GetOrderedComponentIds())
		{
			auto compInstance = entity.GetComponentById(compID);
			// Do not serialize transient component
			auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, compInstance.type());
			if (bDiscardSerialize) continue;

			// Serialize component
			YAML::Node compNode;
			compNode["Component"] = compID;
			ComponentSerializer cs;
			cs.Serialize(compNode, compInstance);
			node["Components"].push_back(compNode);
		}
	}

	void SceneSerializer::DeserializeEntity(const YAML::Node& node, const Ref<Scene>& scene)
	{
		const UUID uuid = node["Entity"].as<UUID>();
		Entity deserializedEntity = scene->CreateEntityWithUUID(uuid);

		if (auto components = node["Components"])
		{
			for (auto component : components)
			{
				const auto compID = component["Component"].as<U32>();
				// TODO: NativeScriptComponent deserialization
				if (compID == entt::type_hash<NativeScriptComponent>::value()) continue;

				auto compInstance = deserializedEntity.GetOrAddComponentById(compID, true);
				// Instance may be null as compID is invalid
				if (compInstance)
				{
					// Deserialize component
					ComponentSerializer cs;
					// We must pass by reference here for component to be deserialized property
					cs.Deserialize(component, compInstance.as_ref());
				}
			}
		}
	}

}
