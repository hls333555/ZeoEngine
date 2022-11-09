#pragma once

#include <optional>

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	class ReflectionUtils
	{
	public:
		static FieldType MetaTypeToFieldType(entt::meta_type type);
		static bool DoesComponentContainAnyField(U32 compID);

		static bool IsComponentRegistered(U32 compID);

		template<typename T>
		static bool IsTypeEqual(entt::meta_type type)
		{
			return type.info().hash() == entt::type_hash<T>::value();
		}

		template<typename T>
		static const char* GetMetaObjectName(T metaObj)
		{
			auto prop = metaObj.prop(Reflection::PropertyType::Name);
			if (prop)
			{
				return prop.value().cast<const char*>();
			}
			return nullptr;
		}

		static const char* GetEnumDisplayName(const entt::meta_any& enumValue);
		static const char* GetComponentDisplayNameFull(U32 compID);

		template<typename T>
		static bool DoesPropertyExist(Reflection::PropertyType propType, T metaObj)
		{
			for (const auto& prop : metaObj.prop())
			{
				if (prop.key() == propType)
				{
					return true;
				}
			}
			return false;
		}

		template<typename Ret, typename T>
		static std::optional<Ret> GetPropertyValue(Reflection::PropertyType propType, T metaObj)
		{
			for (const auto& prop : metaObj.prop())
			{
				if (prop.key() == propType)
				{
					// Make value const so that allow_cast will call the const version
					const auto value = prop.value();
					return value.allow_cast<Ret>().cast<Ret>();
				}
			}
			return {};
		}

		template<typename Func>
		static void ForEachFieldInComponent(entt::meta_type compType, Func func)
		{
			for (const auto data : compType.data())
			{
				func(data);
			}
			for (const auto base : compType.base())
			{
				for (const auto data : base.data())
				{
					func(data);
				}
			}
		}

		static entt::meta_any ConstructComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static void RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static entt::meta_any GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static entt::meta_any HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static void PatchComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static entt::meta_any CopyComponent(entt::meta_type compType, entt::registry& dstRegistry, entt::entity dstEntity, entt::meta_any& compInstance);

	};
	
}
