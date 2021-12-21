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
	struct convert<AssetHandle<Texture2DAsset>>
	{
		static Node encode(const AssetHandle<Texture2DAsset>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, AssetHandle<Texture2DAsset>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = Texture2DAssetLibrary::Get().LoadAsset(path);
			return true;
		}
	};

	template<>
	struct convert<AssetHandle<ParticleTemplateAsset>>
	{
		static Node encode(const AssetHandle<ParticleTemplateAsset>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, AssetHandle<ParticleTemplateAsset>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = ParticleTemplateAssetLibrary::Get().LoadAsset(path);
			return true;
		}
	};

	template<>
	struct convert<AssetHandle<MeshAsset>>
	{
		static Node encode(const AssetHandle<MeshAsset>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, AssetHandle<MeshAsset>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = MeshAssetLibrary::Get().LoadAsset(path);
			return true;
		}
	};

	template<>
	struct convert<AssetHandle<MaterialAsset>>
	{
		static Node encode(const AssetHandle<MaterialAsset>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, AssetHandle<MaterialAsset>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = MaterialAssetLibrary::Get().LoadAsset(path);
			return true;
		}
	};

	template<>
	struct convert<AssetHandle<ShaderAsset>>
	{
		static Node encode(const AssetHandle<ShaderAsset>& rhs)
		{
			Node node;
			node.push_back(rhs ? rhs->GetPath() : "");
			return node;
		}

		static bool decode(const Node& node, AssetHandle<ShaderAsset>& rhs)
		{
			const auto& path = node.as<std::string>();
			if (path.empty()) return true;

			rhs = ShaderAssetLibrary::Get().LoadAsset(path);
			return true;
		}
	};

}

namespace ZeoEngine {

	const char* g_AssetTypeToken = "AssetType";
	const char* g_SourceToken = "SourcePath";

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

	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetHandle<Texture2DAsset>& texture)
	{
		out << (texture ? texture->GetPath() : "");
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetHandle<ParticleTemplateAsset>& pTemplate)
	{
		out << (pTemplate ? pTemplate->GetPath() : "");
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetHandle<MeshAsset>& mesh)
	{
		out << (mesh ? mesh->GetPath() : "");
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetHandle<MaterialAsset>& material)
	{
		out << (material ? material->GetPath() : "");
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const AssetHandle<ShaderAsset>& shader)
	{
		out << (shader ? shader->GetPath() : "");
		return out;
	}

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
				SerializeData<AssetHandle<Texture2DAsset>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::PARTICLE:
				SerializeData<AssetHandle<ParticleTemplateAsset>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::MESH:
				SerializeData<AssetHandle<MeshAsset>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::MATERIAL:
				SerializeData<AssetHandle<MaterialAsset>>(out, data, instance, bIsSeqElement);
				break;
			case BasicMetaType::SHADER:
				SerializeData<AssetHandle<ShaderAsset>>(out, data, instance, bIsSeqElement);
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
				DeserializeData<AssetHandle<Texture2DAsset>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::PARTICLE:
				DeserializeData<AssetHandle<ParticleTemplateAsset>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::MESH:
				DeserializeData<AssetHandle<MeshAsset>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::MATERIAL:
				DeserializeData<AssetHandle<MaterialAsset>>(data, instance, value, bIsSeqElement);
				break;
			case BasicMetaType::SHADER:
				DeserializeData<AssetHandle<ShaderAsset>>(data, instance, value, bIsSeqElement);
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

	void MaterialSerializer::Serialize(YAML::Emitter& out, const Ref<Material>& material)
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
				SerializeData<int32_t>(out, uniform);
				break;
			case ShaderReflectionType::Float:
				SerializeData<float>(out, uniform);
				break;
			case ShaderReflectionType::Vec2:
				SerializeData<glm::vec2>(out, uniform);
				break;
			case ShaderReflectionType::Vec3:
				SerializeData<glm::vec3>(out, uniform);
				break;
			case ShaderReflectionType::Vec4:
				SerializeData<glm::vec4>(out, uniform);
				break;
			case ShaderReflectionType::Texture2D:
				SerializeData<AssetHandle<Texture2DAsset>>(out, uniform);
				break;
			default:
				break;
		}
	}

	void MaterialSerializer::Deserialize(const YAML::Node& value, const Ref<Material>& material)
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
				DeserializeData<int32_t>(value, uniform);
				break;
			case ShaderReflectionType::Float:
				DeserializeData<float>(value, uniform);
				break;
			case ShaderReflectionType::Vec2:
				DeserializeData<glm::vec2>(value, uniform);
				break;
			case ShaderReflectionType::Vec3:
				DeserializeData<glm::vec3>(value, uniform);
				break;
			case ShaderReflectionType::Vec4:
				DeserializeData<glm::vec4>(value, uniform);
				break;
			case ShaderReflectionType::Texture2D:
				DeserializeData<AssetHandle<Texture2DAsset>>(value, uniform);
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
		auto data = ReadDataFromAsset(path, typeId);
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
		WriteDataToAsset(path, typeId, [&](YAML::Emitter& out)
		{
			out << YAML::Key << g_SourceToken << YAML::Value <<
				(resourcePath.empty() ? AssetRegistry::Get().GetPathSpec(path)->GetSourcePath() : resourcePath);

			ComponentSerializer cs;
			cs.Serialize(out, instance);
		});
	}

	bool ImportableAssetSerializer::Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, IAsset* asset)
	{
		auto data = ReadDataFromAsset(path, typeId);
		if (!data) return false;

		auto assetSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(path);
		if (assetSpec)
		{
			auto sourceData = (*data)[g_SourceToken];
			if (sourceData)
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

	void MaterialAssetSerializer::Serialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const Ref<Material>& material)
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

	bool MaterialAssetSerializer::Deserialize(const std::string& path, AssetTypeId typeId, entt::meta_any instance, const Ref<Material>& material)
	{
		auto data = ReadDataFromAsset(path, typeId);
		if (!data) return false;

		// Derialize component data
		ComponentSerializer cs;
		cs.Deserialize(*data, instance);

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
		uint64_t uuid = entity["Entity"].as<uint64_t>();
		Entity deserializedEntity = scene->CreateEntityWithUUID(uuid);

		auto components = entity["Components"];
		if (components)
		{
			for (auto component : components)
			{
				auto compId = component["Component"].as<uint32_t>();
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
		WriteDataToAsset(path, SceneAsset::TypeId(), [&](YAML::Emitter& out)
		{
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			{
				scene->m_Registry.view<CoreComponent>().each([&](auto entityId, auto& cc)
				{
					Entity entity = { entityId, scene };
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
		auto data = ReadDataFromAsset(path, SceneAsset::TypeId());
		if (!data) return false;

		auto entities = (*data)["Entities"];
		if (entities)
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
