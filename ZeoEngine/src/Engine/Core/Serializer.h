#pragma once

#include <yaml-cpp/yaml.h>
#include <deque>

#include "Engine/GameFramework/Entity.h"
#include "Engine/Core/ReflectionHelper.h"
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

	class ComponentSerializer
	{
	public:
		void Serialize(YAML::Node& node, entt::meta_any instance);
		void Deserialize(const YAML::Node& node, entt::meta_any instance);

	private:
		/** Reverse data order so that serialized and deserialized datas are in correct order. */
		void PreprocessDatas(entt::meta_any& instance);

		// NOTE: Changing instance to const& will cause assertion but don't know why...
		void EvaluateSerializeData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement);
		void EvaluateSerializeSequenceContainerData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance);
		void EvaluateSerializeAssociativeContainerData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance);
		void EvaluateSerializeStructData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement);

		template<typename T>
		void SerializeData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement)
		{
			if (bIsSeqElement)
			{
				const auto elementValue = instance.cast<T>();
				if constexpr (std::is_same_v<T, U8>)
				{
					// This '+' can force output U8 as number
					node.push_back(+elementValue);
				}
				else
				{
					node.push_back(elementValue);
				}
			}
			else
			{
				const auto dataName = GetMetaObjectDisplayName(data);
				const auto dataValue = data.get(instance).cast<T>();
				if constexpr (std::is_same_v<T, U8>)
				{
					node[*dataName] = +dataValue;
				}
				else
				{
					node[*dataName] = dataValue;
				}
			}
		}
		void SerializeEnumData(YAML::Node& node, const entt::meta_data data, entt::meta_any& instance, bool bIsSeqElement);

		void EvaluateDeserializeData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement);
		void EvaluateDeserializeSequenceContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& node);
		void EvaluateDeserializeAssociativeContainerData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& node);
		void EvaluateDeserializeStructData(const entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement);

		template<typename T>
		void DeserializeData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement)
		{
			const auto& dataValue = node.as<T>();
			if (bIsSeqElement)
			{
				instance.cast<T&>() = dataValue;
			}
			else
			{
				data.set(instance, dataValue);
			}
		}
		void DeserializeEnumData(entt::meta_data data, entt::meta_any& instance, const YAML::Node& node, bool bIsSeqElement);

	private:
		std::deque<entt::meta_data> m_PreprocessedDatas;
	};

	class MaterialSerializer
	{
	public:
		void Serialize(YAML::Node& node, const Ref<Material>& material);
		void Deserialize(const YAML::Node& node, const Ref<Material>& material);

	private:
		void EvaluateSerializeData(YAML::Node& node, const Ref<DynamicUniformDataBase>& uniform);

		template<typename T>
		void SerializeData(YAML::Node& node, const Ref<DynamicUniformDataBase>& uniform) const
		{
			const auto& dataName = uniform->Name;
			const auto& dataValue = *static_cast<T*>(uniform->GetValuePtr());
			node[dataName] = dataValue;
		}

		void EvaluateDeserializeData(const YAML::Node& node, const Ref<DynamicUniformDataBase>& uniform);

		template<typename T>
		void DeserializeData(const YAML::Node& node, const Ref<DynamicUniformDataBase>& uniform) const
		{
			const auto& dataValue = node.as<T>();
			*static_cast<T*>(uniform->GetValuePtr()) = dataValue;
		}
	};

	class SceneSerializer
	{
	public:
		static void Serialize(YAML::Node& node, const Ref<Scene>& scene);
		static void SerializeRuntime();
		static void Deserialize(const YAML::Node& node, const Ref<Scene>& scene);
		static void DeserializeRuntime();

	private:
		static void SerializeEntity(YAML::Node& node, const Entity entity);
		static void DeserializeEntity(const YAML::Node& node, const Ref<Scene>& scene);
	};

}
