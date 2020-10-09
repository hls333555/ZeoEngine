#pragma once

#include <optional>

#include "entt.hpp"

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
T& get(entt::registry& registry, entt::entity entity)
{
	return registry.template get<T>(entity);
}

enum class PropertyType
{
    Name,					// const char*
    Tooltip,				// const char*
	SetterAndGetter,		// [key_only]

	DragSensitivity,		// float
	ClampMin,				// [type_dependent]
	ClampMax,				// [type_dependent]
};

#define ZE_REFL_TYPE(typeName, ...)													\
entt::meta<typeName>()                                                              \
    .type()                                                                         \
        .prop(PropertyType::Name, #typeName)                                        \
        .prop(std::make_tuple(__VA_ARGS__))											\
	.func<&get<typeName>, entt::as_ref_t>("get"_hs)

#define ZE_REFL_DATA_WITH_POLICY(typeName, dataName, policy, ...)					\
.data<&typeName::dataName, policy>(#dataName##_hs)                                  \
    .prop(PropertyType::Name, #dataName)                                            \
    .prop(std::make_tuple(__VA_ARGS__))
#define ZE_REFL_DATA(typeName, dataName, ...) ZE_REFL_DATA_WITH_POLICY(typeName, dataName, entt::as_is_t, __VA_ARGS__) // Registering data this way cannot directly modify the instance value via editor UI, consider using ZE_REFL_DATA_REF instead
#define ZE_REFL_DATA_REF(typeName, dataName, ...) ZE_REFL_DATA_WITH_POLICY(typeName, dataName, entt::as_ref_t, __VA_ARGS__)
#define ZE_REFL_DATA_SETTER_GETTER(typeName, dataName, setterName, getterName, ...)	\
.data<&typeName::setterName, &typeName::getterName>(#dataName##_hs)					\
    .prop(PropertyType::Name, #dataName)                                            \
    .prop(PropertyType::SetterAndGetter)                                            \
    .prop(std::make_tuple(__VA_ARGS__))

#define ZE_REFL_ENUM(enumTypeName)													\
entt::meta<enumTypeName>()
#define ZE_REFL_ENUM_DATA(enumTypeName, enumDataName)								\
.data<enumTypeName::enumDataName>(#enumDataName##_hs)

#define ZE_REFL_PROP(propType) PropertyType::propType
#define ZE_REFL_PROP_PAIR(propType, propValue) std::make_pair(PropertyType::propType, propValue) // For certain properties with integral type value (e.g. int8_t), you should use ZE_REFL_PROP_PAIR_WITH_CAST instead to explicitly specify their types during property registration
#define ZE_REFL_PROP_PAIR_WITH_CAST(propType, propValue, castToType) std::make_pair(PropertyType::propType, static_cast<castToType>(propValue))

namespace ZeoEngine {

	entt::meta_any GetTypeInstance(entt::meta_type type, entt::registry& registry, entt::entity entity);

	template<typename T>
	bool IsTypeEqual(entt::meta_type type)
	{
		return type.type_id() == entt::type_info<T>::id();
	}

	template<typename T>
	T& GetDataValueByRef(entt::meta_data data, entt::meta_any instance)
	{
		return data.get(instance).cast<T>();
	}

	template<typename T>
	void SetDataValue(entt::meta_data data, entt::meta_any instance, T&& value)
	{
		data.set(instance, std::forward<T>(value));
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

}
