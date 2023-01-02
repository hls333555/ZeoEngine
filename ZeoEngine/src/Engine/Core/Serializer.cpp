#include "ZEpch.h"
#include "Engine/Core/Serializer.h"

#include "Project.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Tags.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Scripting/ScriptFieldInstance.h"
#include "Engine/Utils/ReflectionUtils.h"

namespace ZeoEngine {

	/** Map from component ID to its serializer extender */
	static std::unordered_map<U32, Scope<ComponentSerializerExtenderBase>> s_ComponentSerializerExtenders;

	void ComponentSerializerExtenderRegistry::AddComponentSerializerExtender(U32 compID, Scope<ComponentSerializerExtenderBase> extender)
	{
		s_ComponentSerializerExtenders[compID] = std::move(extender);
	}

	ComponentSerializerExtenderBase* ComponentSerializerExtenderRegistry::GetComponentSerializerExtender(U32 compID)
	{
		return s_ComponentSerializerExtenders.find(compID) != s_ComponentSerializerExtenders.end() ? s_ComponentSerializerExtenders[compID].get() : nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	// ComponentSerializerExtenders //////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ScriptComponentSerializerExtender::Serialize(YAML::Node& compNode, IComponent* comp)
	{
		const auto* scriptComp = static_cast<ScriptComponent*>(comp);
		const auto& className = scriptComp->ClassName;
		if (ScriptEngine::EntityClassExists(className))
		{
			const auto entityID = scriptComp->OwnerEntity.GetUUID();
			auto& entityFields = ScriptEngine::GetScriptFieldMap(entityID);
			for (const auto& [name, field] : entityFields)
			{
				// TODO: Do not serialize transient data

				EvaluateSerializeField(compNode, field, false);
			}
		}
	}

	void ScriptComponentSerializerExtender::Deserialize(const YAML::Node& compNode, IComponent* comp)
	{
		const auto* scriptComp = static_cast<ScriptComponent*>(comp);
		const auto& className = scriptComp->ClassName;
		if (ScriptEngine::EntityClassExists(className))
		{
			const auto entityID = scriptComp->OwnerEntity.GetUUID();
			auto& entityFields = ScriptEngine::GetScriptFieldMap(entityID);
			for (const auto& [name, field] : entityFields)
			{
				const char* fieldName = field->GetFieldName();
				const auto& fieldNode = compNode[fieldName];
				// Evaluate serialized field only
				if (fieldNode)
				{
					EvaluateDeserializeField(fieldNode, field);
				}
			}
		}
	}

	// TODO: Enum and containers...
	void ScriptComponentSerializerExtender::EvaluateSerializeField(YAML::Node& node, const Ref<ScriptFieldInstance>& fieldInstance, bool bIsSeqElement) const
	{
		const auto type = fieldInstance->GetFieldType();
		switch (type)
		{
			case FieldType::Bool:	SerializeField<bool>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::I8:		SerializeField<I8>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::U8:		SerializeField<U8>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::I16:	SerializeField<I16>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::U16:	SerializeField<U16>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::I32:	SerializeField<I32>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::U32:	SerializeField<U32>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::I64:	SerializeField<I64>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::U64:	SerializeField<U64>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Float:	SerializeField<float>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Double:	SerializeField<double>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Vec2:	SerializeField<Vec2>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Vec3:	SerializeField<Vec3>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Vec4:	SerializeField<Vec4>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::String:	SerializeField<std::string>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Asset:	SerializeField<AssetHandle>(node, fieldInstance, bIsSeqElement); break;
			case FieldType::Entity:	SerializeField<UUID>(node, fieldInstance, bIsSeqElement); break;
			default: ZE_CORE_ASSERT(false);
		}
	}

	// TODO: Enum and containers...
	void ScriptComponentSerializerExtender::EvaluateDeserializeField(const YAML::Node& fieldNode, const Ref<ScriptFieldInstance>& fieldInstance) const
	{
		const auto type = fieldInstance->GetFieldType();
		switch (type)
		{
			case FieldType::Bool:	DeserializeField<bool>(fieldNode, fieldInstance); break;
			case FieldType::I8:		DeserializeField<I8>(fieldNode, fieldInstance); break;
			case FieldType::U8:		DeserializeField<U8>(fieldNode, fieldInstance); break;
			case FieldType::I16:	DeserializeField<I16>(fieldNode, fieldInstance); break;
			case FieldType::U16:	DeserializeField<U16>(fieldNode, fieldInstance); break;
			case FieldType::I32:	DeserializeField<I32>(fieldNode, fieldInstance); break;
			case FieldType::U32:	DeserializeField<U32>(fieldNode, fieldInstance); break;
			case FieldType::I64:	DeserializeField<I64>(fieldNode, fieldInstance); break;
			case FieldType::U64:	DeserializeField<U64>(fieldNode, fieldInstance); break;
			case FieldType::Float:	DeserializeField<float>(fieldNode, fieldInstance); break;
			case FieldType::Double:	DeserializeField<double>(fieldNode, fieldInstance); break;
			case FieldType::Vec2:	DeserializeField<Vec2>(fieldNode, fieldInstance); break;
			case FieldType::Vec3:	DeserializeField<Vec3>(fieldNode, fieldInstance); break;
			case FieldType::Vec4:	DeserializeField<Vec4>(fieldNode, fieldInstance); break;
			case FieldType::String:	DeserializeField<std::string>(fieldNode, fieldInstance); break;
			case FieldType::Asset:	DeserializeField<AssetHandle>(fieldNode, fieldInstance); break;
			case FieldType::Entity:	DeserializeField<UUID>(fieldNode, fieldInstance); break;
			default: ZE_CORE_ASSERT(false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ComponentSerializer ///////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ComponentSerializer::Serialize(YAML::Node& compNode, entt::meta_any compInstance)
	{
		if (!compInstance) return;

		const auto preprocessedFields = PreprocessFields(compInstance);
		for (const auto data : preprocessedFields)
		{
			// Do not serialize transient data
			auto bDiscardSerialize = ReflectionUtils::DoesPropertyExist(Reflection::Transient, data);
			if (bDiscardSerialize) continue;

			auto fieldInstance = data.get(compInstance);
			const char* fieldName = ReflectionUtils::GetMetaObjectName(data);
			EvaluateSerializeField(compNode, fieldInstance, fieldName, false);
		}

		if (auto* extender = ComponentSerializerExtenderRegistry::GetComponentSerializerExtender(compInstance.type().id()))
		{
			auto* comp = compInstance.try_cast<IComponent>();
			extender->Serialize(compNode, comp);
		}
	}

	std::deque<entt::meta_data> ComponentSerializer::PreprocessFields(const entt::meta_any& compInstance)
	{
		std::deque<entt::meta_data> preprocessedFields;
		const auto type = compInstance.type();
		ReflectionUtils::ForEachFieldInComponent(type, [&preprocessedFields](entt::meta_data data)
		{
			preprocessedFields.push_front(data);
		});
		return preprocessedFields;
	}

	void ComponentSerializer::EvaluateSerializeField(YAML::Node& node, const entt::meta_any& fieldInstance, const char* fieldName, bool bIsSeqElement)
	{
		const auto type = ReflectionUtils::MetaTypeToFieldType(fieldInstance.type());
		switch (type)
		{
			case FieldType::Bool:	SerializeField<bool>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::I8:		SerializeField<I8>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::U8:		SerializeField<U8>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::I16:	SerializeField<I16>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::U16:	SerializeField<U16>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::I32:	SerializeField<I32>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::U32:	SerializeField<U32>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::I64:	SerializeField<I64>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::U64:	SerializeField<U64>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Float:	SerializeField<float>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Double:	SerializeField<double>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Vec2:	SerializeField<Vec2>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Vec3:	SerializeField<Vec3>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Vec4:	SerializeField<Vec4>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Enum:	SerializeEnumField(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::String:	SerializeField<std::string>(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::SeqCon: EvaluateSerializeSequenceContainerField(node, fieldInstance, fieldName, bIsSeqElement); break;
			case FieldType::Asset:	SerializeField<AssetHandle>(node, fieldInstance, fieldName, bIsSeqElement); break;
			default: ZE_CORE_ASSERT(false);
		}
	}

	void ComponentSerializer::EvaluateSerializeSequenceContainerField(YAML::Node& node, const entt::meta_any& seqInstance, const char* fieldName, bool bIsSeqElement)
	{
		ZE_CORE_ASSERT(!bIsSeqElement);

		auto seqView = seqInstance.as_sequence_container();
		YAML::Node seqNode;
		seqNode.SetStyle(YAML::EmitterStyle::Flow);
		for (auto it = seqView.begin(); it != seqView.end(); ++it)
		{
			auto elementInstance = *it;
			EvaluateSerializeField(seqNode, elementInstance, fieldName, true);
		}
		node[fieldName] = seqNode;
	}

	void ComponentSerializer::SerializeEnumField(YAML::Node& node, const entt::meta_any& enumInstance, const char* fieldName, bool bIsSeqElement)
	{
		const char* enumValueName = ReflectionUtils::GetEnumDisplayName(enumInstance);
		if (bIsSeqElement)
		{
			node.push_back(enumValueName);
		}
		else
		{
			node[fieldName] = enumValueName;
		}
	}

	void ComponentSerializer::Deserialize(const YAML::Node& compNode, entt::meta_any compInstance, Entity entity)
	{
		DeserializeInternal(compNode, compInstance.as_ref(), [&compInstance, &entity](U32 fieldID)
		{
			const U32 compID = compInstance.type().id();
			entity.PatchComponentByID(compID, fieldID);
		});
	}

	void ComponentSerializer::Deserialize(const YAML::Node& compNode, entt::meta_any compInstance, const AssetSerializerBase* assetSerializer)
	{
		DeserializeInternal(compNode, compInstance.as_ref(), [&compInstance, assetSerializer](U32 fieldID)
		{
			auto* comp = compInstance.try_cast<IComponent>();
			assetSerializer->PostFieldDeserialize(comp, fieldID);
		});
	}

	void ComponentSerializer::EvaluateDeserializeField(const YAML::Node& fieldNode, entt::meta_any& fieldInstance)
	{
		const auto type = ReflectionUtils::MetaTypeToFieldType(fieldInstance.type());
		switch (type)
		{
			case FieldType::Bool:	DeserializeField<bool>(fieldNode, fieldInstance); break;
			case FieldType::I8:		DeserializeField<I8>(fieldNode, fieldInstance); break;
			case FieldType::U8:		DeserializeField<U8>(fieldNode, fieldInstance); break;
			case FieldType::I16:	DeserializeField<I16>(fieldNode, fieldInstance); break;
			case FieldType::U16:	DeserializeField<U16>(fieldNode, fieldInstance); break;
			case FieldType::I32:	DeserializeField<I32>(fieldNode, fieldInstance); break;
			case FieldType::U32:	DeserializeField<U32>(fieldNode, fieldInstance); break;
			case FieldType::I64:	DeserializeField<I64>(fieldNode, fieldInstance); break;
			case FieldType::U64:	DeserializeField<U64>(fieldNode, fieldInstance); break;
			case FieldType::Float:	DeserializeField<float>(fieldNode, fieldInstance); break;
			case FieldType::Double:	DeserializeField<double>(fieldNode, fieldInstance); break;
			case FieldType::Vec2:	DeserializeField<Vec2>(fieldNode, fieldInstance); break;
			case FieldType::Vec3:	DeserializeField<Vec3>(fieldNode, fieldInstance); break;
			case FieldType::Vec4:	DeserializeField<Vec4>(fieldNode, fieldInstance); break;
			case FieldType::Enum:	DeserializeEnumField(fieldNode, fieldInstance); break;
			case FieldType::String:	DeserializeField<std::string>(fieldNode, fieldInstance); break;
			case FieldType::SeqCon: EvaluateDeserializeSequenceContainerField(fieldNode, fieldInstance); break;
			case FieldType::Asset:	DeserializeField<AssetHandle>(fieldNode, fieldInstance); break;
			default: ZE_CORE_ASSERT(false);
		}
	}

	void ComponentSerializer::EvaluateDeserializeSequenceContainerField(const YAML::Node& seqNode, entt::meta_any& seqInstance)
	{
		auto seqView = seqInstance.as_sequence_container();
		// Clear elements first
		seqView.clear();
		// TODO: Resize instead of insert every time
		for (const auto& elementNode : seqNode)
		{
			auto it = seqView.insert(seqView.end(), seqView.value_type().construct());
			ZE_CORE_ASSERT(it, "Failed to insert sequence container elements during deserialization! Please check if its type is properly registered.");

			auto elementInstance = *it;
			EvaluateDeserializeField(elementNode, elementInstance);
		}
	}

	void ComponentSerializer::DeserializeEnumField(const YAML::Node& enumNode, entt::meta_any& enumInstance)
	{
		const auto currentName = enumNode.as<std::string>();
		const auto& enumEntries = enumInstance.type().data();
		for (const auto enumData : enumEntries)
		{
			const char* enumEntryName = ReflectionUtils::GetMetaObjectName(enumData);
			if (currentName == enumEntryName)
			{
				const auto value = enumData.get({}).data();
				const U32 size = EngineUtils::GetFieldSize(FieldType::Enum);
				memcpy(enumInstance.data(), value, size);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// MaterialSerializer ////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialSerializer::Serialize(YAML::Node& node, const Ref<Material>& material) const
	{
		if (!material) return;

		for (const auto& field : material->GetDynamicBindableFields())
		{
			EvaluateSerializeField(node, field);
		}
		for (const auto& field : material->GetDynamicFields())
		{
			EvaluateSerializeField(node, field);
		}
	}

	void MaterialSerializer::EvaluateSerializeField(YAML::Node& node, const Ref<DynamicUniformFieldBase>& field) const
	{
		switch (field->GetFieldType())
		{
			case ShaderReflectionFieldType::Bool:		SerializeField<bool>(node, field); break;
			case ShaderReflectionFieldType::Int:		SerializeField<I32>(node, field); break;
			case ShaderReflectionFieldType::Float:		SerializeField<float>(node, field); break;
			case ShaderReflectionFieldType::Vec2:		SerializeField<Vec2>(node, field); break;
			case ShaderReflectionFieldType::Vec3:		SerializeField<Vec3>(node, field); break;
			case ShaderReflectionFieldType::Vec4:		SerializeField<Vec4>(node, field); break;
			case ShaderReflectionFieldType::Texture2D:	SerializeField<AssetHandle>(node, field); break;
			default: ZE_CORE_ASSERT(false);
		}
	}

	void MaterialSerializer::Deserialize(const YAML::Node& node, const Ref<Material>& material) const
	{
		if (!material) return;

		for (const auto& field : material->GetDynamicBindableFields())
		{
			const auto& fieldName = field->Name;
			const auto& fieldNode = node[fieldName];
			// Evaluate serialized data only
			if (fieldNode)
			{
				EvaluateDeserializeData(fieldNode, field);
			}
		}
		for (const auto& field : material->GetDynamicFields())
		{
			const auto& fieldName = field->Name;
			const auto& fieldNode = node[fieldName];
			// Evaluate serialized data only
			if (fieldNode)
			{
				EvaluateDeserializeData(fieldNode, field);
			}
		}
	}

	void MaterialSerializer::EvaluateDeserializeData(const YAML::Node& fieldNode, const Ref<DynamicUniformFieldBase>& field) const
	{
		switch (field->GetFieldType())
		{
			case ShaderReflectionFieldType::Bool:		DeserializeField<bool>(fieldNode, field); break;
			case ShaderReflectionFieldType::Int:		DeserializeField<I32>(fieldNode, field); break;
			case ShaderReflectionFieldType::Float:		DeserializeField<float>(fieldNode, field); break;
			case ShaderReflectionFieldType::Vec2:		DeserializeField<Vec2>(fieldNode, field); break;
			case ShaderReflectionFieldType::Vec3:		DeserializeField<Vec3>(fieldNode, field); break;
			case ShaderReflectionFieldType::Vec4:		DeserializeField<Vec4>(fieldNode, field); break;
			case ShaderReflectionFieldType::Texture2D:	DeserializeField<AssetHandle>(fieldNode, field); break;
			default: ZE_CORE_ASSERT(false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneSerializer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	void SceneSerializer::Serialize(YAML::Node& node, Scene& scene)
	{
		const auto coreView = scene.GetComponentView<CoreComponent>();
		for (const auto e : coreView)
		{
			const Entity entity{ e, scene.shared_from_this() };
			if (!entity) return;

			YAML::Node entityNode;
			SerializeEntity(entityNode, entity);
			node["Entities"].push_back(entityNode);
		}
	}

	void SceneSerializer::SerializeRuntime()
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
	}

	void SceneSerializer::Deserialize(const YAML::Node& node, Scene& scene)
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

	void SceneSerializer::SerializeEntity(YAML::Node& entityNode, Entity entity)
	{
		entityNode["Entity"] = entity.GetUUID();
		// Do not call entt::registry::visit() as the order is reversed
		for (const auto compID : entity.GetRegisteredComponentIDs())
		{
			auto compInstance = entity.GetComponentByID(compID);
			// Do not serialize transient component
			auto bDiscardSerialize = ReflectionUtils::DoesPropertyExist(Reflection::Transient, compInstance.type());
			if (bDiscardSerialize) continue;

			// Serialize component
			YAML::Node compNode;
			compNode["Component"] = compID;
			ComponentSerializer cs;
			cs.Serialize(compNode, compInstance);
			entityNode["Components"].push_back(compNode);
		}
	}

	void SceneSerializer::DeserializeEntity(const YAML::Node& entityNode, Scene& scene)
	{
		const UUID uuid = entityNode["Entity"].as<UUID>();
		Entity deserializedEntity = scene.CreateEntityWithUUID(uuid);
		deserializedEntity.AddTag<Tag::IsDeserializing>();

		if (auto components = entityNode["Components"])
		{
			for (auto component : components)
			{
				const auto compID = component["Component"].as<U32>();
				// TODO: NativeScriptComponent deserialization
				if (compID == entt::type_hash<NativeScriptComponent>::value()) continue;

				auto compInstance = deserializedEntity.GetOrAddComponentByID(compID, true);
				// Instance may be null as compID is invalid
				if (compInstance)
				{
					// Deserialize component
					ComponentSerializer cs;
					// We must pass by reference here for component to be deserialized properly
					cs.Deserialize(component, compInstance.as_ref(), deserializedEntity);
				}
			}
		}

		deserializedEntity.RemoveTag<Tag::IsDeserializing>();
	}

	void ProjectSerializer::Serialize(const std::string& path, const Project& project)
	{
		YAML::Node node;
		{
			const auto& config = project.GetConfig();

			YAML::Node projectNode;
			projectNode["Name"] = config.Name;
			projectNode["AssetDirectory"] = config.AssetDirectory;
			projectNode["ScriptAssemblyDirectory"] = config.ScriptAssemblyDirectory;
			projectNode["DefaultLevelAsset"] = config.DefaultLevelAsset;
			node["Project"] = projectNode;
		}

		std::ofstream fout(path);
		fout << node;
	}

	bool ProjectSerializer::Deserialize(const std::string& path, Project& project)
	{
		YAML::Node node;
		try
		{
			node = YAML::LoadFile(path);
		}
		catch (YAML::BadFile&)
		{
			ZE_CORE_ERROR("Failed to load project: {0}!", path);
			return false;
		}

		{
			auto& config = project.GetConfig();
			auto projectNode = node["Project"];
			config.Name = projectNode["Name"].as<std::string>();
			config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
			config.ScriptAssemblyDirectory = projectNode["ScriptAssemblyDirectory"].as<std::string>();
			config.DefaultLevelAsset = projectNode["DefaultLevelAsset"].as<AssetHandle>();
		}

		return true;
	}

}
