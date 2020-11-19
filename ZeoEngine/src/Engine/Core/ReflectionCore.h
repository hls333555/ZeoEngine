#pragma once

#include <entt.hpp>

namespace ZeoReflection {

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
	bool has(entt::registry& registry, entt::entity entity) // NOTE: Do not register it by ref!
	{
		return registry.template has<T>(entity);
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

}

enum class PropertyType
{
	Name,						// [value_type: const char*] Name of type or data.
	Tooltip,					// [value_type: const char*] Tooltip of type or data.
	AsCopy,						// [key_only] Indicates this data is registered using a copy. This should accord with entt's policy.
	HideTypeHeader,				// [key_only] This type will not display CollapsingHeader.
	NestedClass,				// [key_only] This type has subdatas and will display a special TreeNode.

	DragSensitivity,			// [value_type: float] Speed of dragging.
	ClampMin,					// [value_type: type_dependent] Min value.
	ClampMax,					// [value_type: type_dependent] Max value.
	ClampOnlyDuringDragging,	// [key_only] Should value be clamped only during dragging? If this property is not set, inputted value will not get clamped.

	DisplayName,				// [value_type: const char*] Display name of type or data.
	InherentType,				// [key_only] This type cannot be added or removed within editor.
	Category,					// [value_type: const char*] Category of type or data.
	HiddenInEditor,				// [key_only] Should hide this data in editor?
	HideCondition,				// [value_type: const char*] Hide this data if provided expression yields true. Supported types: bool and enum. Supported operators: == and !=.
	Transient,					// [key_only] If set, this data will not get serialized.
};

#define ZE_TEXT(text) u8##text
#define ZE_DATA_ID(data) #data##_hs

//////////////////////////////////////////////////////////////////////////
// Registration Header ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ZE_CAT_IMPL(a, b) a##b
#define ZE_CAT(a, b) ZE_CAT_IMPL(a, b)
#define ZE_REFL_REGISTRATION																			\
static void ze_auto_register_reflection_function_();													\
namespace																								\
{																										\
    struct ze__auto__register__																			\
    {																									\
        ze__auto__register__()																			\
        {																								\
            ze_auto_register_reflection_function_();													\
        }																								\
    };																									\
}																										\
static const ze__auto__register__ ZE_CAT(auto_register__, __LINE__);									\
static void ze_auto_register_reflection_function_()

//////////////////////////////////////////////////////////////////////////
// Component Registration ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ZE_REFL_COMPONENT(_type, ...)																	\
entt::meta<_type>()																						\
    .type()																								\
        .prop(PropertyType::Name, #_type)																\
        .prop(std::make_tuple(__VA_ARGS__))																\
		.ctor<&ZeoReflection::emplace<_type>, entt::as_ref_t>()											\
		.func<&ZeoReflection::get<_type>, entt::as_ref_t>("get"_hs)										\
		.func<&ZeoReflection::remove<_type>, entt::as_ref_t>("remove"_hs)								\
		.func<&ZeoReflection::has<_type>>("has"_hs)														\
		.func<&_type::OnDestroy>("on_destroy"_hs)														\
		.func<&Component::OnDataValueEditChange>("OnDataValueEditChange"_hs)							\
		.func<&Component::PostDataValueEditChange>("PostDataValueEditChange"_hs)						\
		.base<Component>()

//////////////////////////////////////////////////////////////////////////
// Nested Type Registration //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ZE_REFL_NESTED_TYPE(_type, ...)																	\
entt::meta<_type>()																						\
    .type()																								\
        .prop(PropertyType::Name, #_type)																\
        .prop(PropertyType::InherentType)																\
        .prop(PropertyType::NestedClass)																\
        .prop(std::make_tuple(__VA_ARGS__))																\
		.ctor<&ZeoReflection::emplace<_type>, entt::as_ref_t>()											\
		.func<&ZeoReflection::get<_type>, entt::as_ref_t>("get"_hs)										\
		.func<&ZeoReflection::remove<_type>, entt::as_ref_t>("remove"_hs)								\
		.func<&ZeoReflection::has<_type>>("has"_hs)														\
		.func<&ZeoReflection::create_default_value<_type>>("create_default_value"_hs)

//////////////////////////////////////////////////////////////////////////
// Data Registration /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define INTERNAL_ZE_REFL_DATA_WITH_POLICY(_type, _data, policy, ...)									\
.data<&_type::_data, policy>(#_data##_hs)																\
    .prop(PropertyType::Name, #_data)																	\
    .prop(std::make_tuple(__VA_ARGS__))
#define ZE_REFL_DATA(_type, _data, ...) INTERNAL_ZE_REFL_DATA_WITH_POLICY(_type, _data, entt::as_is_t, __VA_ARGS__).prop(PropertyType::AsCopy)
#define ZE_REFL_DATA_REF(_type, _data, ...) INTERNAL_ZE_REFL_DATA_WITH_POLICY(_type, _data, entt::as_ref_t, __VA_ARGS__)
#define INTERNAL_ZE_REFL_DATA_SETTER_GETTER_WITH_POLICY(_type, dataName, setter, getter, policy, ...)	\
.data<setter, getter, policy>(#dataName##_hs)															\
    .prop(PropertyType::Name, #dataName)																\
    .prop(std::make_tuple(__VA_ARGS__))
#define ZE_REFL_DATA_SETTER_GETTER(_type, dataName, setterName, getterName, ...) INTERNAL_ZE_REFL_DATA_SETTER_GETTER_WITH_POLICY(_type, dataName, &_type::setterName, &_type::getterName, entt::as_is_t, __VA_ARGS__).prop(PropertyType::AsCopy) // Please use ZE_REFL_DATA_SETTER_GETTER_REF instead to register getter and setter for containers
#define ZE_REFL_DATA_SETTER_GETTER_REF(_type, dataName, getterName, ...) INTERNAL_ZE_REFL_DATA_SETTER_GETTER_WITH_POLICY(_type, dataName, nullptr, &_type::getterName, entt::as_ref_t, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
// Enum Registration /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ZE_REFL_ENUM(enumType)																			\
entt::meta<enumType>()																					\
	.func<&ZeoReflection::set_enum_value<enumType>, entt::as_ref_t>("set_enum_value"_hs)				\
	.func<&ZeoReflection::create_default_value<enumType>>("create_default_value"_hs)
#define ZE_REFL_ENUM_DATA(enumType, enumData, ...)														\
.data<enumType::enumData>(#enumData##_hs)																\
    .prop(PropertyType::Name, #enumData)																\
	.prop(std::make_tuple(__VA_ARGS__))

//////////////////////////////////////////////////////////////////////////
// Property Registration /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ZE_REFL_PROP(propType) PropertyType::propType
#define ZE_REFL_PROP_PAIR(propType, propValue) std::make_pair(PropertyType::propType, propValue)
