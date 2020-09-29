#pragma once

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
    Name,
    Tooltip,

	DragSensitivity,
	Min,
	Max,
};

#define ZE_REFL_TYPE(typeName, tooltip)                                             \
entt::meta<typeName>()                                                              \
    .type()                                                                         \
        .prop(PropertyType::Name, #typeName)                                        \
        .prop(PropertyType::Tooltip, u8##tooltip)									\
	.func<&get<typeName>, entt::as_ref_t>("get"_hs)

#define ZE_REFL_DATA_WITH_POLICY(typeName, dataName, tooltip, policy)               \
.data<&typeName::dataName, policy>(#dataName##_hs)                                  \
    .prop(PropertyType::Name, #dataName)                                            \
    .prop(PropertyType::Tooltip, u8##tooltip)                                                  
#define ZE_REFL_DATA(typeName, dataName, tooltip) ZE_REFL_DATA_WITH_POLICY(typeName, dataName, tooltip, entt::as_is_t) // Registering data this way cannot directly modify the instance value via editor UI, consider using ZE_REFL_DATA_REF instead
#define ZE_REFL_DATA_REF(typeName, dataName, tooltip) ZE_REFL_DATA_WITH_POLICY(typeName, dataName, tooltip, entt::as_ref_t)

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

	template<typename Ret, typename T>
	Ret GetPropData(PropertyType propType, T metaObj)
	{
		return metaObj.prop(propType).value().cast<Ret>();
	}

}
