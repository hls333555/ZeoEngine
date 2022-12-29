#pragma once

#include <yaml-cpp/yaml.h>
#include <deque>

#include "Engine/GameFramework/Entity.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/Material.h"

namespace YAML {

	using namespace ZeoEngine;

	template<>
	struct convert<Vec2>
	{
		static Node encode(const Vec2& rhs)
		{
			Node node;
			node.SetStyle(EmitterStyle::Flow);
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
			node.SetStyle(EmitterStyle::Flow);
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
			node.SetStyle(EmitterStyle::Flow);
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

	template<>
	struct convert<std::filesystem::path>
	{
		static Node encode(const std::filesystem::path& rhs)
		{
			Node node;
			node = rhs.string();
			return node;
		}

		static bool decode(const Node& node, std::filesystem::path& rhs)
		{
			rhs = node.as<std::string>();
			return true;
		}
	};

	template<>
	struct convert<AssetHandle>
	{
		static Node encode(const AssetHandle& rhs)
		{
			Node node;
			node = static_cast<U64>(rhs);
			return node;
		}

		static bool decode(const Node& node, AssetHandle& rhs)
		{
			rhs = node.as<U64>();
			return true;
		}
	};

}

namespace ZeoEngine {

	class ScriptFieldInstance;
	class Project;

	class ComponentSerializerExtenderRegistry
	{
	public:
		template<typename ComponentSerializerExtender, typename Component>
		static void RegisterComponentSerializerExtender()
		{
			AddComponentSerializerExtender(entt::type_hash<Component>::value(), CreateScope<ComponentSerializerExtender>());
		}
		static class ComponentSerializerExtenderBase* GetComponentSerializerExtender(U32 compID);

	private:
		static void AddComponentSerializerExtender(U32 compID, Scope<ComponentSerializerExtenderBase> extender);
	};

	class ComponentSerializerExtenderBase
	{
	public:
		virtual ~ComponentSerializerExtenderBase() = default;

		virtual void Serialize(YAML::Node& compNode, IComponent* comp) = 0;
		virtual void Deserialize(const YAML::Node& compNode, IComponent* comp) = 0;
	};

	class ScriptComponentSerializerExtender : public ComponentSerializerExtenderBase
	{
	public:
		virtual void Serialize(YAML::Node& compNode, IComponent* comp) override;
		virtual void Deserialize(const YAML::Node& compNode, IComponent* comp) override;

	private:
		void EvaluateSerializeField(YAML::Node& node, const Ref<ScriptFieldInstance>& fieldInstance, bool bIsSeqElement) const;
		void EvaluateDeserializeField(const YAML::Node& fieldNode, const Ref<ScriptFieldInstance>& fieldInstance) const;

	private:
		template<typename T>
		static void SerializeField(YAML::Node& node, const Ref<ScriptFieldInstance>& fieldInstance, bool bIsSeqElement)
		{
			const auto value = fieldInstance->GetValue<T>();
			const char* fieldName = fieldInstance->GetFieldName();
			if (bIsSeqElement)
			{
				if constexpr (std::is_same_v<T, U8>)
				{
					// This '+' can force output U8 as number
					node.push_back(+value);
				}
				else
				{
					node.push_back(value);
				}
			}
			else
			{
				if constexpr (std::is_same_v<T, U8>)
				{
					node[fieldName] = +value;
				}
				else
				{
					node[fieldName] = value;
				}
			}
		}

		template<typename T>
		void DeserializeField(const YAML::Node& fieldNode, const Ref<ScriptFieldInstance>& fieldInstance) const
		{
			const auto& value = fieldNode.as<T>();
			fieldInstance->SetValue(value);
		}
	};

	class ComponentSerializer
	{
	public:
		void Serialize(YAML::Node& compNode, entt::meta_any compInstance);
		void Deserialize(const YAML::Node& compNode, entt::meta_any compInstance, Entity entity);
		void Deserialize(const YAML::Node& compNode, entt::meta_any compInstance, const AssetSerializerBase* assetSerializer);

	private:
		template<typename Func>
		void DeserializeInternal(const YAML::Node& compNode, entt::meta_any compInstance, Func func)
		{
			if (!compInstance) return;

			const auto preprocessedFields = PreprocessFields(compInstance);
			for (const auto data : preprocessedFields)
			{
				const char* fieldName = ReflectionUtils::GetMetaObjectName(data);
				const auto& fieldNode = compNode[fieldName];
				// Evaluate serialized field only
				if (fieldNode)
				{
					auto fieldInstance = data.get(compInstance);
					EvaluateDeserializeField(fieldNode, fieldInstance);
					func(data.id());
				}
			}

			if (auto* extender = ComponentSerializerExtenderRegistry::GetComponentSerializerExtender(compInstance.type().id()))
			{
				auto* comp = compInstance.try_cast<IComponent>();
				extender->Deserialize(compNode, comp);
			}
		}

		/** Reverse order so that serialized and deserialized fields are in correct order. */
		std::deque<entt::meta_data> PreprocessFields(const entt::meta_any& compInstance);

		void EvaluateSerializeField(YAML::Node& node, const entt::meta_any& fieldInstance, const char* fieldName, bool bIsSeqElement);
		void EvaluateSerializeSequenceContainerField(YAML::Node& node, const entt::meta_any& seqInstance, const char* fieldName, bool bIsSeqElement);

		template<typename T>
		void SerializeField(YAML::Node& node, const entt::meta_any& fieldInstance, const char* fieldName, bool bIsSeqElement)
		{
			const auto value = fieldInstance.cast<T>();
			if (bIsSeqElement)
			{
				if constexpr (std::is_same_v<T, U8>)
				{
					// This '+' can force output U8 as number
					node.push_back(+value);
				}
				else
				{
					node.push_back(value);
				}
			}
			else
			{
				if constexpr (std::is_same_v<T, U8>)
				{
					node[fieldName] = +value;
				}
				else
				{
					node[fieldName] = value;
				}
			}
		}
		void SerializeEnumField(YAML::Node& node, const entt::meta_any& enumInstance, const char* fieldName, bool bIsSeqElement);

		void EvaluateDeserializeField(const YAML::Node& fieldNode, entt::meta_any& fieldInstance);
		void EvaluateDeserializeSequenceContainerField(const YAML::Node& seqNode, entt::meta_any& seqInstance);

		template<typename T>
		void DeserializeField(const YAML::Node& fieldNode, entt::meta_any& fieldInstance) const
		{
			const auto& value = fieldNode.as<T>();
			fieldInstance.cast<T&>() = value;
		}
		void DeserializeEnumField(const YAML::Node& enumNode, entt::meta_any& enumInstance);
	};

	class MaterialSerializer
	{
	public:
		void Serialize(YAML::Node& node, const Ref<Material>& material) const;
		void Deserialize(const YAML::Node& node, const Ref<Material>& material) const;

	private:
		void EvaluateSerializeField(YAML::Node& node, const Ref<DynamicUniformFieldBase>& field) const;

		template<typename T>
		void SerializeField(YAML::Node& node, const Ref<DynamicUniformFieldBase>& field) const
		{
			const auto& fieldName = field->Name;
			const auto& value = *static_cast<T*>(field->GetValueRaw());
			node[fieldName] = value;
		}

		void EvaluateDeserializeData(const YAML::Node& fieldNode, const Ref<DynamicUniformFieldBase>& field) const;

		template<typename T>
		void DeserializeField(const YAML::Node& fieldNode, const Ref<DynamicUniformFieldBase>& field) const
		{
			const auto& value = fieldNode.as<T>();
			*static_cast<T*>(field->GetValueRaw()) = value;
		}
	};

	class SceneSerializer
	{
	public:
		static void Serialize(YAML::Node& node, Scene& scene);
		static void SerializeRuntime();
		static void Deserialize(const YAML::Node& node, Scene& scene);
		static void DeserializeRuntime();

	private:
		static void SerializeEntity(YAML::Node& entityNode, Entity entity);
		static void DeserializeEntity(const YAML::Node& entityNode, Scene& scene);
	};

	class ProjectSerializer
	{
	public:
		static void Serialize(const std::string& path, const Project& project);
		static bool Deserialize(const std::string& path, Project& project);
	};

}
