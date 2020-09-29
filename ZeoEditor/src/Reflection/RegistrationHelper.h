#pragma once

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

enum class PropertyType
{
    Name,
    Tooltip,
};

#define ZE_REFL_TYPE(typeName, tooltip)                                             \
entt::meta<typeName>()                                                              \
    .type()                                                                         \
        .prop(PropertyType::Name, #typeName)                                        \
        .prop(PropertyType::Tooltip, u8##tooltip)                                             

#define ZE_REFL_DATA_WITH_POLICY(typeName, dataName, tooltip, policy)               \
.data<&typeName::dataName, policy>(#dataName##_hs)                                  \
    .prop(PropertyType::Name, #dataName)                                            \
    .prop(PropertyType::Tooltip, u8##tooltip)                                                  
#define ZE_REFL_DATA(typeName, dataName, tooltip) ZE_REFL_DATA_WITH_POLICY(typeName, dataName, tooltip, entt::as_is_t)
#define ZE_REFL_DATA_REF(typeName, dataName, tooltip) ZE_REFL_DATA_WITH_POLICY(typeName, dataName, tooltip, entt::as_ref_t)

template<typename Type>
bool IsTypeEqual(entt::meta_type type)
{
    return type.type_id() == entt::type_info<Type>::id();
}

template<typename T>
const char* GetPropName(T metaObj)
{
    return metaObj.prop(PropertyType::Name).value().cast<const char*>();
}

template<typename T>
const char* GetPropTooltip(T metaObj)
{
	return metaObj.prop(PropertyType::Tooltip).value().cast<const char*>();
}
