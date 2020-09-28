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

#define ZE_PROP_NAME "PropName"_hs

#define ZE_REFL_COMP(component)                                                     \
entt::meta<component>()                                                             \
    .type()                                                                         \
        .prop(ZE_PROP_NAME, #component)

#define ZE_REFL_DATA_WITH_POLICY(component, dataName, policy)                       \
.data<&component::dataName, policy>(#dataName##_hs)                                 \
    .prop(ZE_PROP_NAME, #dataName)
#define ZE_REFL_DATA(component, dataName) ZE_REFL_DATA_WITH_POLICY(component, dataName, entt::as_is_t)
#define ZE_REFL_DATA_REF(component, dataName) ZE_REFL_DATA_WITH_POLICY(component, dataName, entt::as_ref_t)

#define GET_PROP_NAME(metaObj) metaObj.prop(ZE_PROP_NAME).value().cast<const char*>()
#define IS_TYPE_EQUAL(metaType, type) metaType.type_id() == entt::type_info<type>::id()
