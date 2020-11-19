#pragma once

#include <optional>
#include <deque>

#include "Engine/Core/ReflectionCore.h"

namespace entt {

	/**
	 * @brief Meta sequence container traits for `std::deque`s of any type.
	 * @tparam Type The type of elements.
	 * @tparam Args Other arguments.
	 */
	template<typename Type, typename... Args>
	struct meta_sequence_container_traits<std::deque<Type, Args...>>
		: internal::container_traits<
		std::deque<Type, Args...>,
		internal::basic_container,
		internal::basic_dynamic_container,
		internal::basic_sequence_container,
		internal::dynamic_sequence_container
		>
	{};

}

namespace ZeoEngine {

	const char* GetEnumDisplayName(entt::meta_any enumValue);

	void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue);

	entt::meta_any CreateTypeDefaultValue(entt::meta_type type);

	template<typename T>
	bool IsTypeEqual(entt::meta_type type)
	{
		return type.type_id() == entt::type_info<T>::id();
	}

	template<typename T>
	T& GetDataValueByRef(entt::meta_data data, entt::meta_handle instance)
	{
		return data.get(std::move(instance)).cast<T>();
	}

	template<typename T>
	T GetDataValue(entt::meta_data data, entt::meta_handle instance)
	{
		return data.get(std::move(instance)).cast<T>();
	}

	template<typename T>
	void SetDataValue(entt::meta_data data, entt::meta_handle instance, T&& value)
	{
		data.set(std::move(instance), std::forward<T>(value));
	}

	template<typename T>
	bool DoesPropExist(PropertyType propType, T metaObj)
	{
		return static_cast<bool>(metaObj.prop(propType));
	}

	template<typename Ret, typename T>
	std::optional<Ret> GetPropValue(PropertyType propType, T metaObj)
	{
		const auto prop = metaObj.prop(propType);
		if (prop)
		{
			const auto value = prop.value();
			// As int32_t conversions are already registered, the system can convert to all registered integral types seamlessly
			return value.convert<Ret>().cast<Ret>();
		}
		return {};
	}

	template<typename T>
	std::optional<const char*> GetMetaObjectDisplayName(T metaObj)
	{
		auto prop = metaObj.prop(PropertyType::DisplayName);
		if (prop)
		{
			return prop.value().cast<const char*>();
		}
		else
		{
			prop = metaObj.prop(PropertyType::Name);
			if (prop)
			{
				return prop.value().cast<const char*>();
			}
		}
		return {};
	}

}
