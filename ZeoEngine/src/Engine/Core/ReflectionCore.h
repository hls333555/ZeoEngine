#pragma once

#include <deque>

#include <entt.hpp>

namespace entt {

	/**
	 * @brief Meta sequence container traits for `std::deque`s of any type.
	 * @tparam Type The type of elements.
	 * @tparam Args Other arguments.
	 */
	template<typename Type, typename... Args>
	struct meta_sequence_container_traits<std::deque<Type, Args...>>
		: meta_container_traits<
		std::deque<Type, Args...>,
		basic_container,
		basic_dynamic_container,
		basic_sequence_container,
		dynamic_sequence_container
		>
	{};

}

namespace ZeoEngine::Reflection {

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
	void on_destroy(entt::registry& registry, entt::entity entity)
	{
		registry.get<T>(entity).OnDestroy();
	}

	template<typename T>
	void bind_on_destroy(entt::registry& registry)
	{
		registry.on_destroy<T>().template connect<&on_destroy<T>>();
	}

	template<typename T>
	void set_enum_value_for_seq(entt::meta_any& instance, const entt::meta_any& newValue)
	{
		instance.cast<T&>() = newValue.cast<T>();
	}

}

enum class PropertyType
{
	Name,						// [value_type: const char*] Name of component or data.
	DisplayName,				// [value_type: const char*] Display name of component or data.
	Inherent,					// [key_only] This component cannot be added or removed within editor.
	Tooltip,					// [value_type: const char*] Tooltip of component or data.
	Struct,						// [key_only] This data has subdatas and will display a special TreeNode.
	HideComponentHeader,		// [key_only] This component will not display a collapsing header.
	Category,					// [value_type: const char*] Category of component or data.
	HiddenInEditor,				// [key_only] Should hide this data in editor?
	HideCondition,				// [value_type: const char*] Hide this data if provided expression yields true. Supported types: bool and enum. Supported operators: == and !=.
	Transient,					// [key_only] If set, this data will not get serialized.

	DragSensitivity,			// [value_type: float] Speed of dragging.
	ClampMin,					// [value_type: type_dependent] Min value.
	ClampMax,					// [value_type: type_dependent] Max value.
	ClampOnlyDuringDragging,	// [key_only] Should value be clamped only during dragging? If this property is not set, inputted value will not get clamped.

};

#define ZE_TEXT(text) u8##text
using namespace entt::literals;
#define ZE_DATA_ID(data) #data##_hs

//////////////////////////////////////////////////////////////////////////////
// IMPORTANT NOTE ////////////////////////////////////////////////////////////
// You should do all component registrations inside ZE_REFL_REGISTRATION{...}!
// For more details, you can refer to TestComponent and its registration code.
// Almost all supported types are listed in TestComponent.
//////////////////////////////////////////////////////////////////////////////

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

/**
 * Register component.
 * @param _type - Component type
 * @param ... - Various component properties. Use ZE_REFL_PROP() or ZE_REFL_PROP_PAIR() to register them
 */
#define ZE_REFL_COMPONENT(_type, ...)																	\
entt::meta<_type>()																						\
    .type()																								\
        .prop(PropertyType::Name, #_type)																\
        .prop(std::make_tuple(__VA_ARGS__))																\
		.ctor<&ZeoEngine::Reflection::emplace<_type>, entt::as_ref_t>()									\
		.func<&ZeoEngine::Reflection::remove<_type>, entt::as_ref_t>("remove"_hs)						\
		.func<&ZeoEngine::Reflection::get<_type>, entt::as_ref_t>("get"_hs)								\
		.func<&ZeoEngine::Reflection::has<_type>>("has"_hs)												\
		.func<&ZeoEngine::Reflection::bind_on_destroy<_type>>("bind_on_destroy"_hs)						\
		.base<Component>()

//////////////////////////////////////////////////////////////////////////
// Enum Registration /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register enum (class). Don't forget to register its data members using ZE_REFL_ENUM_DATA()
 * @param enumType - Enum (class) type
 */
#define ZE_REFL_ENUM(enumType)																			\
entt::meta<enumType>()																					\
	.ctor<>()																							\
	.func<&ZeoEngine::Reflection::set_enum_value_for_seq<enumType>, entt::as_ref_t>("set_enum_value_for_seq"_hs)

/**
 * Register enum data members.
 * @param enumType - Enum (class) type
 * @param enumData - Enum data member
 * @param ... - Various enum properties. Use ZE_REFL_PROP() or ZE_REFL_PROP_PAIR() to register them
 */
#define ZE_REFL_ENUM_DATA(enumType, enumData, ...)														\
.data<enumType::enumData>(#enumData##_hs)																\
    .prop(PropertyType::Name, #enumData)																\
	.prop(std::make_tuple(__VA_ARGS__))

//////////////////////////////////////////////////////////////////////////
// Custom Struct Registration ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register custom struct.
 * The struct MUST have ==operator overloading like below:
 * 
 * @code
 * bool operator==(const TestStruct1& other) const // <- This "const" is required!
 * {
 *     ...
 * }
 * @endcode
 * 
 * @param _type - Struct type
 * @param ... - Various struct properties. Use ZE_REFL_PROP() or ZE_REFL_PROP_PAIR() to register them
 */
#define ZE_REFL_STRUCT(_type, ...)																		\
entt::meta<_type>()																						\
    .type()																								\
        .prop(PropertyType::Name, #_type)																\
        .prop(PropertyType::Inherent)																	\
        .prop(PropertyType::Struct)																		\
        .prop(std::make_tuple(__VA_ARGS__))																\
		.ctor<>()

//////////////////////////////////////////////////////////////////////////
// Data Registration /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register component/struct data members (by reference).
 * @param _type - Component/struct type
 * @param _data - Component/struct data member
 * @param ... - Various data properties. Use ZE_REFL_PROP() or ZE_REFL_PROP_PAIR() to register them
 */
#define ZE_REFL_DATA(_type, _data, ...)																	\
.data<&_type::_data, entt::as_ref_t>(#_data##_hs)														\
    .prop(PropertyType::Name, #_data)																	\
    .prop(std::make_tuple(__VA_ARGS__))

/**
 * Register component/struct data members (by value).
 * The getter and setter should look like below (the return value should not be reference type):
 * 
 * @code
 * float GetTestFloat() const { return TestFloat; }
 * void SetTestFloat(float value) { TestFloat = value; }
 * @endcode
 *
 * NOTE: If you want to register getter-setter for containers and custom structs, you should use ZE_REFL_DATA_GETTER_REF() instead!
 * @param _type - Component/struct type
 * @param _dataName - Name of component/struct data member
 * @param _setterName - Name of setter of component/struct data member
 * @param _getterName - Name of getter of component/struct data member
 * @param ... - Various data properties. Use ZE_REFL_PROP() or ZE_REFL_PROP_PAIR() to register them
 */
#define ZE_REFL_DATA_SETTER_GETTER(_type, dataName, setterName, getterName, ...)						\
.data<&_type::setterName, &_type::getterName, entt::as_is_t>(#dataName##_hs)							\
    .prop(PropertyType::Name, #dataName)																\
    .prop(std::make_tuple(__VA_ARGS__))

/**
 * Register component/struct data members (by reference).
 * The getter should look like below (the return value should be reference type and function is non-const):
 * 
 * @code
 * TestStruct& GetTestStruct() { return TestStruct; }
 * auto& GetTestContainer() { return TestContainer; }
 * @endcode
 * 
 * @param _type - Component/struct type
 * @param _dataName - Name of component/struct data member
 * @param _getterName - Name of getter of component/struct data member
 * @param ... - Various data properties. Use ZE_REFL_PROP() or ZE_REFL_PROP_PAIR() to register them
*/
#define ZE_REFL_DATA_GETTER_REF(_type, dataName, getterName, ...)										\
.data<nullptr, &_type::getterName, entt::as_ref_t>(#dataName##_hs)										\
    .prop(PropertyType::Name, #dataName)																\
    .prop(std::make_tuple(__VA_ARGS__))

//////////////////////////////////////////////////////////////////////////
// Property Registration /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register key-only property.
 * @param propType - see enum PropertyType
 */
#define ZE_REFL_PROP(propType) PropertyType::propType

/**
 * Register key-value-pair property.
 * NOTE: const char* values should be wrapped with ZE_TEXT() like below:
 * 
 * @code
 * ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Test"))
 * @endcode
 * 
 * @param propType - see enum PropertyType
 * @param propValue - see enum PropertyType
 */
#define ZE_REFL_PROP_PAIR(propType, propValue) std::make_pair(PropertyType::propType, propValue)
