#pragma once

#include <optional>

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	class ReflectionUtils
	{
	public:
		static Reflection::BasicMetaType EvaluateType(entt::meta_type type);
		static bool DoesTypeContainData(entt::id_type typeId);

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
		static const char* GetComponentDisplayNameFull(U32 compId);

		template<typename T>
		static bool DoesPropertyExist(Reflection::PropertyType propType, T metaObj)
		{
			return static_cast<bool>(metaObj.prop(propType));
		}

		template<typename Ret, typename T>
		static std::optional<Ret> GetPropertyValue(Reflection::PropertyType propType, T metaObj)
		{
			auto prop = metaObj.prop(propType);
			if (prop)
			{
				const auto value = prop.value();
				// As I32 conversions are already registered, the system can convert to all registered integral types seamlessly
				return value.allow_cast<Ret>().cast<Ret>();
			}
			return {};
		}

		static entt::meta_any ConstructComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static void RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static entt::meta_any GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static entt::meta_any HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		static entt::meta_any CopyComponent(entt::meta_type compType, entt::registry& dstRegistry, entt::entity dstEntity, entt::meta_any& compInstance);
		static void BindOnComponentDestroy(entt::meta_type compType, entt::registry& registry);
		static void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue);
	};
	
}
