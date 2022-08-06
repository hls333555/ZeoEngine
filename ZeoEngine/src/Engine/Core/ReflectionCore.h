#pragma once

#include <entt.hpp>

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

	enum class BasicMetaType
	{
		NONE,
		STRUCT, // Custom struct
		SEQCON, // Sequence container
		ASSCON, // Associative container
		BOOL, I8, I32, I64, UI8, UI32, UI64,
		FLOAT, DOUBLE,
		ENUM,
		STRING, VEC2, VEC3, VEC4,
		TEXTURE, PARTICLE, MESH, MATERIAL, SHADER,
	};

	enum PropertyType
	{
		Name,						// [value_type: const char*] Name of component or data.
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

	template<typename T>
	T& emplace(entt::registry& registry, entt::entity entity)
	{
		return registry.emplace<T>(entity);
	}
	template<typename T>
	void remove(entt::registry& registry, entt::entity entity)
	{
		registry.remove<T>(entity);
	}
	template<typename T>
	T& get(entt::registry& registry, entt::entity entity)
	{
		return registry.get<T>(entity);
	}
	template<typename T>
	bool has(entt::registry& registry, entt::entity entity) // NOTE: Do not register it by ref!
	{
		return registry.all_of<T>(entity);
	}

	template<typename T>
	T& copy(entt::registry& dstRegistry, entt::entity dstEntity, const entt::meta_any& comp)
	{
		return dstRegistry.emplace_or_replace<T>(dstEntity, comp.cast<T>());
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
