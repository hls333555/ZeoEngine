#include "ZEpch.h"
#include "Engine/Core/Serializer.h"

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/AssetRegistry.h"

namespace YAML {

	using namespace ZeoEngine;

	template<>
	struct convert<Vec2>
	{
		static Node encode(const Vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, Vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vec3>
	{
		static Node encode(const Vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, Vec3& rhs)
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
	struct convert<Vec4>
	{
		static Node encode(const Vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, Vec4& rhs)
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

	Emitter& operator<<(Emitter& out, const Entity& e)
	{
		out << e.GetEntityId();
		return out;
	}

	Emitter& operator<<(Emitter& out, const Vec2& v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << EndSeq;
		return out;
	}

	Emitter& operator<<(Emitter& out, const Vec3& v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << EndSeq;
		return out;
	}

	Emitter& operator<<(Emitter& out, const Vec4& v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
		return out;
	}

#define DEFINE_ASSET_CONVERTOR(assetClass)									\
	template<>																\
	struct convert<AssetHandle<assetClass>>									\
	{																		\
		static Node encode(const AssetHandle<assetClass>& rhs)				\
		{																	\
			Node node;														\
			node.push_back(rhs ? rhs->GetID() : "");						\
			return node;													\
		}																	\
		static bool decode(const Node& node, AssetHandle<assetClass>& rhs)	\
		{																	\
			const auto& path = node.as<std::string>();						\
			if (path.empty()) return true;									\
			rhs = ZE_CAT(assetClass, Library)::Get().LoadAsset(path);		\
			return true;													\
		}																	\
	};																		\
	Emitter& operator<<(Emitter& out, const AssetHandle<assetClass>& asset)	\
	{																		\
		out << (asset ? asset->GetID() : "");								\
		return out;															\
	}

	DEFINE_ASSET_CONVERTOR(Texture2D);
	DEFINE_ASSET_CONVERTOR(ParticleTemplate);
	DEFINE_ASSET_CONVERTOR(Mesh);
	DEFINE_ASSET_CONVERTOR(Material);
	DEFINE_ASSET_CONVERTOR(Shader);

}

namespace ZeoEngine {

	const char* g_AssetTypeToken = "AssetType";
	const char* g_SourceToken = "SourcePath";

	//////////////////////////////////////////////////////////////////////////
	// ComponentSerializer ///////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ComponentSerializer::Serialize(YAML::Emitter& out, entt::meta_any& instance)
	{
		if (!instance) return;

		PreprocessDatas(instance);

		for (const auto data : m_PreprocessedDatas)
		{
			// Do not serialize transient data
			auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, data);
			if (bDiscardSerialize) continue;

			EvaluateSerializeData(out, data, instance, false);
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

	void ComponentSerializer::EvaluateSerializeData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
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
				SerializeData<I8>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::I32:
				SerializeData<I32>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::I64:
				SerializeData<I64>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::UI8:
				SerializeData<U8>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::UI32:
				SerializeData<U32>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::UI64:
				SerializeData<U64>(out, data, instance, bIsSeqElement);
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
				SerializeData<Vec2>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::VEC3:
				SerializeData<Vec3>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::VEC4:
				SerializeData<Vec4>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::TEXTURE:
				SerializeData<AssetHandle<Texture2D>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::PARTICLE:
				SerializeData<AssetHandle<ParticleTemplate>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::MESH:
				SerializeData<AssetHandle<Mesh>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::MATERIAL:
				SerializeData<AssetHandle<Material>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::SHADER:
				SerializeData<AssetHandle<Shader>>(out, data, instance, bIsSeqElement);
				break;
			default:
				auto dataName = GetMetaObjectDisplayName(data);
				ZE_CORE_ASSERT(false, "Failed to serialize data: '{0}'", *dataName);
				break;
		}
	}

	void ComponentSerializer::EvaluateSerializeSequenceContainerData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance)
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

	void ComponentSerializer::EvaluateSerializeAssociativeContainerData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance)
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
	}

	void ComponentSerializer::EvaluateSerializeStructData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
	{
		const auto dataName = GetMetaObjectDisplayName(data);
		bIsSeqElement ? out << YAML::BeginSeq : out << YAML::Key << *dataName << YAML::Value << YAML::BeginSeq;
		{
			const auto structType = bIsSeqElement ? instance.type() : data.type();
			auto structInstance = bIsSeqElement ? instance.as_ref() : data.get(instance); // NOTE: We must call as_ref() to return reference here or it will return a copy since entt 3.7.0
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

	void ComponentSerializer::SerializeEnumData(YAML::Emitter& out, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
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

	void ComponentSerializer::Deserialize(const YAML::Node& value, entt::meta_any& instance)
	{
		if (!instance) return;

		PreprocessDatas(instance);

		for (const auto data : m_PreprocessedDatas)
		{
			auto dataName = GetMetaObjectDisplayName(data);
			const auto& dataValue = value[*dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(data, instance, dataValue, false);
				IComponent* comp = instance.try_cast<IComponent>();
				if (comp->ComponentHelper)
				{
					comp->ComponentHelper->PostDataDeserialize(data.id());
				}
			}
		}
	}

	void ComponentSerializer::EvaluateDeserializeData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
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
				DeserializeData<I8>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::I32:
				DeserializeData<I32>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::I64:
				DeserializeData<I64>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::UI8:
				DeserializeData<U8>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::UI32:
				DeserializeData<U32>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::UI64:
				DeserializeData<U64>(data, instance, value, bIsSeqElement);
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
				DeserializeData<Vec2>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::VEC3:
				DeserializeData<Vec3>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::VEC4:
				DeserializeData<Vec4>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::TEXTURE:
				DeserializeData<AssetHandle<Texture2D>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::PARTICLE:
				DeserializeData<AssetHandle<ParticleTemplate>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::MESH:
				DeserializeData<AssetHandle<Mesh>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::MATERIAL:
				DeserializeData<AssetHandle<Material>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::SHADER:
				DeserializeData<AssetHandle<Shader>>(data, instance, value, bIsSeqElement);
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

	void ComponentSerializer::EvaluateDeserializeSequenceContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& value)
	{
		auto seqView = data.get(instance).as_sequence_container();
		// Clear elements first
		seqView.clear();
		for (const auto& elementValue : value)
		{
			auto it = InsertDefaultValueForSeq(data, seqView);
			auto elementInstance = *it;
			EvaluateDeserializeData(data, elementInstance, elementValue, true);
		}
	}

	void ComponentSerializer::EvaluateDeserializeAssociativeContainerData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value)
	{

	}

	void ComponentSerializer::EvaluateDeserializeStructData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
	{
		const auto type = bIsSeqElement ? instance.type() : data.type();
		auto structInstance = bIsSeqElement ? instance.as_ref() : data.get(instance); // NOTE: We must call as_ref() to return reference here or it will return a copy since entt 3.7.0
		U32 i = 0;
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

	void ComponentSerializer::DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& value, bool bIsSeqElement)
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
	// MaterialSerializer ////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialSerializer::Serialize(YAML::Emitter& out, const AssetHandle<Material>& material)
	{
		if (!material) return;

		for (const auto& uniform : material->GetDynamicBindableUniforms())
		{
			EvaluateSerializeData(out, uniform);
		}
		for (const auto& uniform : material->GetDynamicUniforms())
		{
			EvaluateSerializeData(out, uniform);
		}
	}

	void MaterialSerializer::EvaluateSerializeData(YAML::Emitter& out, const Ref<DynamicUniformDataBase>& uniform)
	{
		switch (uniform->GetDataType())
		{
			case ShaderReflectionType::Bool:
				SerializeData<bool>(out, uniform);
				break;
			case ShaderReflectionType::Int:
				SerializeData<I32>(out, uniform);
				break;
			case ShaderReflectionType::Float:
				SerializeData<float>(out, uniform);
				break;
			case ShaderReflectionType::Vec2:
				SerializeData<Vec2>(out, uniform);
				break;
			case ShaderReflectionType::Vec3:
				SerializeData<Vec3>(out, uniform);
				break;
			case ShaderReflectionType::Vec4:
				SerializeData<Vec4>(out, uniform);
				break;
			case ShaderReflectionType::Texture2D:
				SerializeData<AssetHandle<Texture2D>>(out, uniform);
				break;
			default:
				break;
		}
	}

	void MaterialSerializer::Deserialize(const YAML::Node& value, const AssetHandle<Material>& material)
	{
		if (!material) return;

		for (const auto& uniform : material->GetDynamicBindableUniforms())
		{
			const auto& dataName = uniform->Name;
			const auto& dataValue = value[dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(dataValue, uniform);
			}
		}
		for (const auto& uniform : material->GetDynamicUniforms())
		{
			const auto& dataName = uniform->Name;
			const auto& dataValue = value[dataName];
			// Evaluate serialized data only
			if (dataValue)
			{
				EvaluateDeserializeData(dataValue, uniform);
			}
		}
	}

	void MaterialSerializer::EvaluateDeserializeData(const YAML::Node& value, const Ref<DynamicUniformDataBase>& uniform)
	{
		switch (uniform->GetDataType())
		{
			case ShaderReflectionType::Bool:
				DeserializeData<bool>(value, uniform);
				break;
			case ShaderReflectionType::Int:
				DeserializeData<I32>(value, uniform);
				break;
			case ShaderReflectionType::Float:
				DeserializeData<float>(value, uniform);
				break;
			case ShaderReflectionType::Vec2:
				DeserializeData<Vec2>(value, uniform);
				break;
			case ShaderReflectionType::Vec3:
				DeserializeData<Vec3>(value, uniform);
				break;
			case ShaderReflectionType::Vec4:
				DeserializeData<Vec4>(value, uniform);
				break;
			case ShaderReflectionType::Texture2D:
				DeserializeData<AssetHandle<Texture2D>>(value, uniform);
				break;
			default:
				break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// AssetSerializer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	void AssetSerializer::Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance)
	{
		WriteDataToAsset(path, typeId, [&](YAML::Emitter& out)
		{
			ComponentSerializer cs;
			cs.Serialize(out, instance);
		});
	}

	bool AssetSerializer::Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance)
	{
		const auto data = ReadDataFromAsset(path, typeId);
		if (!data) return false;

		ComponentSerializer cs;
		cs.Deserialize(*data, instance);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// ImportableAssetSerializer /////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImportableAssetSerializer::Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const std::string& resourcePath)
	{
		const std::string& assetPath = path;
		WriteDataToAsset(assetPath, typeId, [&](YAML::Emitter& out)
		{
			out << YAML::Key << g_SourceToken << YAML::Value <<
				(resourcePath.empty() ? AssetRegistry::Get().GetPathSpec(assetPath)->GetSourcePath() : resourcePath);

			ComponentSerializer cs;
			cs.Serialize(out, instance);
		});
	}

	bool ImportableAssetSerializer::Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance)
	{
		const std::string& assetPath = path;
		auto data = ReadDataFromAsset(assetPath, typeId);
		if (!data) return false;

		if (const auto assetSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(assetPath))
		{
			if (const auto sourceData = (*data)[g_SourceToken])
			{
				assetSpec->SourcePath = sourceData.as<std::string>();
			}
		}
		ComponentSerializer cs;
		cs.Deserialize(*data, instance);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaterialAssetSerializer ///////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialAssetSerializer::Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const AssetHandle<Material>& material)
	{
		WriteDataToAsset(path, typeId, [&](YAML::Emitter& out)
		{
			// Serialize component data
			ComponentSerializer cs;
			cs.Serialize(out, instance);

			// Serialize shader uniform data
			MaterialSerializer ms;
			ms.Serialize(out, material);
		});
	}

	bool MaterialAssetSerializer::Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const AssetHandle<Material>& material, bool bIncludeComponentData)
	{
		const auto data = ReadDataFromAsset(path, typeId);
		if (!data) return false;

		// Derialize component data
		if (bIncludeComponentData)
		{
			ComponentSerializer cs;
			cs.Deserialize(*data, instance);
		}

		// Derialize shader uniform data
		MaterialSerializer ms;
		ms.Deserialize(*data, material);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneSerializer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static void SerializeEntity(YAML::Emitter& out, const Entity entity)
	{
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();
			out << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
			{
				// Do not call entt::registry::visit() as the order is reversed
				for (const auto compId : entity.GetOrderedComponentIds())
				{
					auto compInstance = entity.GetComponentById(compId);
					// Do not serialize transient component
					auto bDiscardSerialize = DoesPropExist(PropertyType::Transient, compInstance.type());
					if (bDiscardSerialize) continue;

					out << YAML::BeginMap;
					{
						out << YAML::Key << "Component" << YAML::Value << compId; // TODO: Component ID goes here
						ComponentSerializer cs;
						cs.Serialize(out, compInstance);
					}
					out << YAML::EndMap;
				}
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;
	}

	static void DeserializeEntity(const YAML::Node& entity, const Ref<Scene>& scene)
	{
		U64 uuid = entity["Entity"].as<U64>();
		Entity deserializedEntity = scene->CreateEntityWithUUID(uuid);

		if (auto components = entity["Components"])
		{
			for (auto component : components)
			{
				auto compId = component["Component"].as<U32>();
				// TODO: NativeScriptComponent deserialization
				if (compId == entt::type_hash<NativeScriptComponent>::value()) continue;

				entt::meta_any compInstance = deserializedEntity.GetOrAddComponentById(compId, true);
				// Instance may be null as compId is invalid
				if (compInstance)
				{
					ComponentSerializer cs;
					cs.Deserialize(component, compInstance);
				}
			}
		}
	}

	void SceneSerializer::Serialize(const std::string& path, const Ref<Scene>& scene)
	{
		WriteDataToAsset(path, Level::TypeId(), [&](YAML::Emitter& out)
		{
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			{
				scene->m_Registry.view<CoreComponent>().each([&](auto entityId, auto& cc)
				{
					const Entity entity = { entityId, scene };
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

	bool SceneSerializer::Deserialize(const std::string& path, const Ref<Scene>& scene)
	{
		auto data = ReadDataFromAsset(path, Level::TypeId());
		if (!data) return false;

		if (auto entities = (*data)["Entities"])
		{
			for (auto entity : entities)
			{
				DeserializeEntity(entity, scene);
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

}
