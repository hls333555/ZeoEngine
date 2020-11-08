#pragma once

#include <optional>

#include <deque>

#include <entt.hpp>

#include "Engine/GameFramework/Entity.h"

#define ZE_CAT_IMPL(a, b) a##b
#define ZE_CAT(a, b) ZE_CAT_IMPL(a, b)
#define ZE_REFL_REGISTRATION                                                        \
static void ze_auto_register_reflection_function_();                                \
namespace                                                                           \
{                                                                                   \
    struct ze__auto__register__                                                     \
    {                                                                               \
        ze__auto__register__()                                                      \
        {                                                                           \
            ze_auto_register_reflection_function_();                                \
        }                                                                           \
    };                                                                              \
}                                                                                   \
static const ze__auto__register__ ZE_CAT(auto_register__, __LINE__);                \
static void ze_auto_register_reflection_function_()

template<typename T>
T& emplace(entt::registry& registry, entt::entity entity)
{
	return registry.template emplace<T>(entity);
}

template<typename T>
void remove(entt::registry& registry, entt::entity entity)
{
	registry.template remove<T>(entity);
}

template<typename T>
T& get(entt::registry& registry, entt::entity entity)
{
	return registry.template get<T>(entity);
}

template<typename T>
void set_enum_value(entt::meta_any& instance, const entt::meta_any& newValue)
{
	instance.cast<T>() = newValue.cast<T>();
}

template<typename T>
T create_default_value()
{
	return T();
}

enum class PropertyType
{
    Name,					// [value_type: const char*] Name of type or data.
    Tooltip,				// [value_type: const char*] Tooltip of type or data.
	AsCopy,					// [key_only] Indicates this data is registered using a copy. This should accord with entt's policy.
	HideTypeHeader,			// [key_only] This type will not display CollapsingHeader.
	NestedClass,			// [key_only] This type has subdatas and will display a special TreeNode.

	DragSensitivity,		// [value_type: float] Speed of dragging.
	ClampMin,				// [value_type: type_dependent] Min value.
	ClampMax,				// [value_type: type_dependent] Max value.
	ClampOnlyDuringDragging,// [key_only] Should value be clamped only during dragging? If this property is not set, inputted value will not get clamped.

	DisplayName,			// [value_type: const char*] Display name of type or data.
	InherentType,			// [key_only] This type cannot be added or removed within editor.
	HiddenInEditor,			// [key_only] Should hide this data in editor?
	Category,				// [value_type: const char*] Category of type or data.
	HideCondition,			// [value_type: const char*] Hide this data if provided expression yields true.
	Transient,				// [key_only] If set, this data will not get serialized.
};

#define ZE_TEXT(text) u8##text

#define ZE_REFL_TYPE(_type, ...)																\
entt::meta<_type>()																				\
    .type()																						\
        .prop(PropertyType::Name, #_type)														\
        .prop(std::make_tuple(__VA_ARGS__))														\
	.ctor<&emplace<_type>, entt::as_ref_t>()													\
	.func<&get<_type>, entt::as_ref_t>("get"_hs)												\
	.func<&remove<_type>, entt::as_ref_t>("remove"_hs)
#define ZE_REFL_TYPE_NESTED(_type, ...)															\
entt::meta<_type>()																				\
    .type()																						\
        .prop(PropertyType::Name, #_type)														\
        .prop(PropertyType::InherentType)														\
        .prop(PropertyType::NestedClass)														\
        .prop(std::make_tuple(__VA_ARGS__))														\
	.ctor<&emplace<_type>, entt::as_ref_t>()													\
	.func<&get<_type>, entt::as_ref_t>("get"_hs)												\
	.func<&remove<_type>, entt::as_ref_t>("remove"_hs)											\
	.func<&create_default_value<_type>>("create_default_value"_hs)

#define ZE_REFL_DATA_WITH_POLICY(_type, _data, policy, ...)										\
.data<&_type::_data, policy>(#_data##_hs)														\
    .prop(PropertyType::Name, #_data)															\
    .prop(std::make_tuple(__VA_ARGS__))
#define ZE_REFL_DATA(_type, _data, ...) ZE_REFL_DATA_WITH_POLICY(_type, _data, entt::as_is_t, __VA_ARGS__).prop(PropertyType::AsCopy)
#define ZE_REFL_DATA_REF(_type, _data, ...) ZE_REFL_DATA_WITH_POLICY(_type, _data, entt::as_ref_t, __VA_ARGS__)
#define ZE_REFL_DATA_SETTER_GETTER_WITH_POLICY(_type, dataName, setter, getter, policy, ...)	\
.data<setter, getter, policy>(#dataName##_hs)													\
    .prop(PropertyType::Name, #dataName)														\
    .prop(std::make_tuple(__VA_ARGS__))
#define ZE_REFL_DATA_SETTER_GETTER(_type, dataName, setterName, getterName, ...) ZE_REFL_DATA_SETTER_GETTER_WITH_POLICY(_type, dataName, &_type::setterName, &_type::getterName, entt::as_is_t, __VA_ARGS__).prop(PropertyType::AsCopy) // Please use ZE_REFL_DATA_SETTER_GETTER_REF instead to register getter and setter for containers
#define ZE_REFL_DATA_SETTER_GETTER_REF(_type, dataName, getterName, ...) ZE_REFL_DATA_SETTER_GETTER_WITH_POLICY(_type, dataName, nullptr, &_type::getterName, entt::as_ref_t, __VA_ARGS__)

#define ZE_REFL_ENUM(enumType)																	\
entt::meta<enumType>()																			\
	.func<&set_enum_value<enumType>, entt::as_ref_t>("set_enum_value"_hs)						\
	.func<&create_default_value<enumType>>("create_default_value"_hs)
#define ZE_REFL_ENUM_DATA(enumType, enumData, ...)												\
.data<enumType::enumData>(#enumData##_hs)														\
    .prop(PropertyType::Name, #enumData)														\
	.prop(std::make_tuple(__VA_ARGS__))

#define ZE_REFL_PROP(propType) PropertyType::propType
#define ZE_REFL_PROP_PAIR(propType, propValue) std::make_pair(PropertyType::propType, propValue) // For certain properties with integral type value (e.g. int8_t), you should use ZE_REFL_PROP_PAIR_WITH_CAST instead to explicitly specify their types during property registration
#define ZE_REFL_PROP_PAIR_WITH_CAST(propType, propValue, castToType) std::make_pair(PropertyType::propType, static_cast<castToType>(propValue))

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

	entt::meta_any GetTypeInstance(entt::meta_type type, entt::registry& registry, entt::entity entity);

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
		auto prop = metaObj.prop(propType);
		if (prop)
		{
			return prop.value().cast<Ret>();
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
