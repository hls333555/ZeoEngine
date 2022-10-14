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

	// Properties are bound to certain component and do not support inheritance
	enum PropertyType
	{
		Name,						// [value_type: const char*] Name of component or field.
		Inherent,					// [key_only, non-inheritable] This component cannot be added or removed within the editor.
		Tooltip,					// [value_type: const char*] Tooltip of component or field.
		Struct,						// [key_only] This field has sub-fields and will display a special TreeNode.
		HideComponentHeader,		// [key_only] This component will not display the collapsing header.
		Category,					// [value_type: const char*] Category of component or field.
		HiddenInEditor,				// [key_only] Should hide this field in the editor?
		HideCondition,				// [value_type: const char*] Hide this field if provided expression yields true. Supported types: bool and enum. Supported operators: == and !=.
		Transient,					// [key_only] If set, this component or data will not get serialized.

		DragSensitivity,			// [value_type: float] Speed of dragging.
		ClampMin,					// [value_type: type_dependent] Min value.
		ClampMax,					// [value_type: type_dependent] Max value.
		ClampOnlyDuringDragging,	// [key_only] Should value be clamped only during dragging? If this property is not set, input value will not get clamped.
		FixedSizeContainer,			// [key_only] Containers are fixed size so that adding or erasing elements are not allowed.
		CustomElementName,			// [key_only] Container's element name are retrieved from ComponentHelper.
		AssetType,					// [value_type: AssetTypeID] Type ID of asset.

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
		T& comp = registry.get<T>(entity);
		if (auto* helper = ComponentHelperRegistry::GetComponentHelper(entt::type_hash<T>::value()))
		{
			helper->OnComponentDestroy(&comp);
		}
	}

	template<typename T>
	void bind_on_destroy(entt::registry& registry)
	{
		registry.on_destroy<T>().template connect<&on_destroy<T>>();
	}

}
