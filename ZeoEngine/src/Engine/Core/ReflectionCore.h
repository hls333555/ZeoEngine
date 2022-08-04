#pragma once

#include <entt.hpp>

#include "Engine/Core/CoreMacros.h"

#if 0
#include <deque>
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
#endif

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
		return registry.template all_of<T>(entity);
	}

	template<typename T>
	T& copy(entt::registry& dstRegistry, entt::entity dstEntity, const entt::meta_any& comp)
	{
		return dstRegistry.template emplace_or_replace<T>(dstEntity, comp.cast<T>());
	}

	template<typename T>
	void on_destroy(entt::registry& registry, entt::entity entity)
	{
		registry.get<T>(entity).ComponentHelper->OnComponentDestroy();
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
	Transient,					// [key_only] If set, this component or data will not get serialized.

	DragSensitivity,			// [value_type: float] Speed of dragging.
	ClampMin,					// [value_type: type_dependent] Min value.
	ClampMax,					// [value_type: type_dependent] Max value.
	ClampOnlyDuringDragging,	// [key_only] Should value be clamped only during dragging? If this property is not set, inputted value will not get clamped.
	FixedSizeContainer,			// [key_only] Containers are fixed size so that adding or erasing elements are not allowed.
	CustomElementName,			// [key_only] Container's element name are retrieved from ComponentHelper.

};

#define MONO_COMP_NAME(comp) ZE_STRINGIFY(ZeoEngine.comp)
#define REGISTER_MONO_COMP(comp)																	\
if (auto* monoType = mono_reflection_type_from_name(MONO_COMP_NAME(comp), ScriptEngine::GetCoreAssemblyImage()))\
{																									\
	ScriptRegistry::s_RegisteredMonoComponents[monoType] = entt::type_hash<comp>::value();			\
}

#define ZTEXT(text) ZE_CAT(u8, text)
using namespace entt::literals;
#define ZDATA_ID(data) ZE_CAT(ZE_STRINGIFY(data), _hs)

#define _ZPROP_SHARED(_type, ...)																	\
.props(std::make_pair(PropertyType::Name, ZE_STRINGIFY(_type)), std::make_tuple(__VA_ARGS__))
#define _ZPROP_STRUCT(_type, ...)																	\
.props(std::make_pair(PropertyType::Name, ZE_STRINGIFY(_type)), PropertyType::Inherent, PropertyType::Struct, std::make_tuple(__VA_ARGS__))

//////////////////////////////////////////////////////////////////////////////
// IMPORTANT NOTE ////////////////////////////////////////////////////////////
// You should do all component registrations inside ZREGISTRATION{...}!
// For more details, you can refer to TestComponent and its registration code.
// Almost all supported types are listed in TestComponent.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Registration Header ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ZE_REGISTRATION																				\
static void ze_auto_register_reflection_function_();												\
namespace																							\
{																									\
    struct ze__auto__register__																		\
    {																								\
        ze__auto__register__()																		\
        {																							\
            ze_auto_register_reflection_function_();												\
        }																							\
    };																								\
}																									\
static const ze__auto__register__ ZE_CAT(auto_register__, __LINE__);								\
static void ze_auto_register_reflection_function_()

//////////////////////////////////////////////////////////////////////////
// Component Registration ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register component.
 * Please register its data using ZDATA().
 * @param _comp - Component type
 * @param ... - Various component properties. Use ZPROP() to register them
 */
#define ZCOMPONENT(_comp, ...)																		\
REGISTER_MONO_COMP(_comp)																			\
entt::meta<_comp>()																					\
    .type()																							\
        _ZPROP_SHARED(_comp, __VA_ARGS__)															\
		.ctor<&ZeoEngine::Reflection::emplace<_comp>, entt::as_ref_t>()								\
		.func<&ZeoEngine::Reflection::remove<_comp>, entt::as_void_t>("remove"_hs)					\
		.func<&ZeoEngine::Reflection::get<_comp>, entt::as_ref_t>("get"_hs)							\
		.func<&ZeoEngine::Reflection::has<_comp>>("has"_hs)											\
		.func<&ZeoEngine::Reflection::copy<_comp>, entt::as_ref_t>("copy"_hs)						\
		.func<&ZeoEngine::Reflection::bind_on_destroy<_comp>, entt::as_void_t>("bind_on_destroy"_hs)\
		.func<&_comp::GetIcon>("get_icon"_hs)														\
		.base<IComponent>()

//////////////////////////////////////////////////////////////////////////
// Enum Registration /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register enum/enum class.
 * Please register its data using ZENUM_DATA()
 * @param _enum - Enum/enum class type
 */
#define ZENUM(_enum)																				\
entt::meta<_enum>()																					\
	.func<&ZeoEngine::Reflection::set_enum_value_for_seq<_enum>, entt::as_void_t>("set_enum_value_for_seq"_hs)

//////////////////////////////////////////////////////////////////////////
// Custom Struct Registration ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register custom struct.
 * The struct MUST have == operator overloading like below:
 * 
 * @code
 * bool operator==(const TestStruct1& other) const // <- This "const" is required!
 * {
 *     ...
 * }
 * @endcode
 * 
 * @param _struct - Struct type
 * @param ... - Various struct properties. Use ZPROP() to register them
 */
#define ZSTRUCT(_struct, ...)																		\
entt::meta<_struct>()																				\
    .type()																							\
	    _ZPROP_STRUCT(_struct, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
// Data Registration /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/**
 * Register component/struct data members (by reference).
 * @param _type - Component/struct type
 * @param _data - Component/struct data member
 * @param ... - Various data properties. Use ZPROP() to register them
 */
#define ZDATA(_type, _data, ...)																	\
.data<&_type::_data, entt::as_ref_t>(ZDATA_ID(_data))												\
	_ZPROP_SHARED(_data, __VA_ARGS__)

/**
 * Register component/struct data members (by value).
 * The getter and setter should look like below (the return value should not be reference type):
 * 
 * @code
 * float GetTestFloat() const { return TestFloat; }
 * void SetTestFloat(float value) { TestFloat = value; }
 * @endcode
 *
 * NOTE: If you want to register getter-setter for containers and custom structs, you should use ZDATA_GETTER_REF() instead!
 * @param _type - Component/struct type
 * @param _data - Component/struct data member
 * @param _setter - Setter of component/struct data member
 * @param _getter - Getter of component/struct data member
 * @param ... - Various data properties. Use ZPROP() to register them
 */
#define ZDATA_SETTER_GETTER(_type, _data, _setter, _getter, ...)									\
.data<&_type::_setter, &_type::_getter, entt::as_is_t>(ZDATA_ID(_data))								\
	_ZPROP_SHARED(_data, __VA_ARGS__)

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
 * @param _data - Component/struct data member
 * @param _getter - Getter of component/struct data member
 * @param ... - Various data properties. Use ZPROP() to register them
*/
#define ZDATA_GETTER_REF(_type, _data, _getter, ...)												\
.data<nullptr, &_type::_getter, entt::as_ref_t>(ZDATA_ID(_data))									\
	_ZPROP_SHARED(_data, __VA_ARGS__)

/**
 * Register enum data members.
 * @param _enum - Enum/enum class type
 * @param _data - Enum data member
 * @param ... - Various enum properties. Use ZPROP() to register them
 */
#define ZENUM_DATA(_enum, _data, ...)																\
.data<_enum::_data>(ZDATA_ID(_data))																\
	_ZPROP_SHARED(_data, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
// Property Registration /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define _ZPROP_ONE_PARAM(_prop) PropertyType::_prop
#define _ZPROP_TWO_PARAMS(_prop, _value) std::make_pair(PropertyType::_prop, _value)
#define _ZPROP_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define _ZPROP_GET_MACRO(...) ZE_EXPAND( _ZPROP_GET_MACRO_NAME(__VA_ARGS__, _ZPROP_TWO_PARAMS, _ZPROP_ONE_PARAM) )
/**
 * Register a key-only property or a key-value-pair property.
 * See enum PropertyType for details.
 * NOTE: const char* values should be wrapped with ZTEXT().
 */
#define ZPROP(...) ZE_EXPAND( _ZPROP_GET_MACRO(__VA_ARGS__)(__VA_ARGS__) )
