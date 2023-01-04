#pragma once

#include <entt.hpp>

#include "Engine/GameFramework/Scene.h"

using namespace entt::literals;

namespace entt {

    /** Custom mixin support our scene as registry. */
    template<typename Type>
    class zeo_sigh_storage_mixin final : public Type {
        using basic_iterator = typename Type::basic_iterator;

        template<typename Func>
        void notify_destruction(basic_iterator first, basic_iterator last, Func func) {
            ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");

            for (; first != last; ++first) {
                const auto entt = *first;
                destruction.publish(*owner, entt);
                const auto it = Type::find(entt);
                func(it, it + 1u);
            }
        }

        void swap_and_pop(basic_iterator first, basic_iterator last) final {
            notify_destruction(std::move(first), std::move(last), [this](auto... args) { Type::swap_and_pop(args...); });
        }

        void in_place_pop(basic_iterator first, basic_iterator last) final {
            notify_destruction(std::move(first), std::move(last), [this](auto... args) { Type::in_place_pop(args...); });
        }

        basic_iterator try_emplace(const typename Type::entity_type entt, const bool force_back, const void* value) final {
            ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
            Type::try_emplace(entt, force_back, value);
            construction.publish(*owner, entt);
            return Type::find(entt);
        }

    public:
        using entity_type = typename Type::entity_type;

        using Type::Type;

        [[nodiscard]] auto on_construct() ENTT_NOEXCEPT {
            return sink{ construction };
        }

        [[nodiscard]] auto on_update() ENTT_NOEXCEPT {
            return sink{ update };
        }

        [[nodiscard]] auto on_destroy() ENTT_NOEXCEPT {
            return sink{ destruction };
        }

        template<typename... Args>
        decltype(auto) emplace(const entity_type entt, Args &&...args) {
            ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
            Type::emplace(entt, std::forward<Args>(args)...);
            construction.publish(*owner, entt);
            return this->get(entt);
        }

        template<typename... Func>
        decltype(auto) patch(const entity_type entt, Func &&...func) {
            ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
            Type::patch(entt, std::forward<Func>(func)...);
            update.publish(*owner, entt);
            return this->get(entt);
        }

        template<typename It, typename... Args>
        void insert(It first, It last, Args &&...args) {
            ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
            Type::insert(first, last, std::forward<Args>(args)...);

            for (auto it = construction.empty() ? last : first; it != last; ++it) {
                construction.publish(*owner, *it);
            }
        }

        void bind(any value) ENTT_NOEXCEPT final {
            auto* reg = static_cast<ZeoEngine::Scene*>(any_cast<registry>(&value));
            owner = reg ? reg : owner;
            Type::bind(std::move(value));
        }

    private:
        sigh<void(ZeoEngine::Scene&, const entity_type)> construction{};
        sigh<void(ZeoEngine::Scene&, const entity_type)> destruction{};
        sigh<void(ZeoEngine::Scene&, const entity_type)> update{};
        ZeoEngine::Scene* owner{};
    };

	template<typename Type>
	struct storage_traits<entity, Type> {
		using storage_type =zeo_sigh_storage_mixin<basic_storage<entity, Type>>;
	};

}

namespace ZeoEngine {

    using HideConditionFunc = bool(*)(struct IComponent* component);
    using CustomSequenceContainerElementNameFunc = std::string(*)(struct IComponent* component, U32 fieldID, U32 elementIndex);
    using CustomWidgetConstructFunc = Scope<class IFieldWidget> (*)(U32 widgetID, Ref<class ComponentFieldInstance> fieldInstance);

    namespace Reflection {

        // Properties are bound to certain component and do not support inheritance
        enum PropertyType
        {
            Name,						// [value_type: const char*] Name of component or field.
            Inherent,					// [key_only] This component cannot be added or removed within the editor.
            Tooltip,					// [value_type: const char*] Tooltip of component or field.
            Struct,						// [key_only] This field has sub-fields and will display a special TreeNode.
            HideComponentHeader,		// [key_only] This component will not display the collapsing header.
            Category,					// [value_type: const char*] Category of component or field.
            HiddenInEditor,				// [key_only] Should hide this field in the editor?
            HideCondition,				// [value_type: HideConditionFunc] Hide this field if provided function returns true.
            Transient,					// [key_only] If set, this component or data will not get serialized.

            DragSensitivity,			// [value_type: float] Speed of dragging.
            ClampMin,					// [value_type: type_dependent] Min value.
            ClampMax,					// [value_type: type_dependent] Max value.
            ClampOnlyDuringDragging,	// [key_only] Should value be clamped only during dragging? If this property is not set, input value will not get clamped.
            FixedSizeContainer,			// [key_only] Containers are fixed size so that adding or erasing elements are not allowed.
            CustomElementName,			// [value_type: CustomSequenceContainerElementNameFunc] Container's element name are retrieved from a free function.
            CustomWidget,			    // [value_type: CustomWidgetConstructFunc] Custom field widget construction function. The returned widget class should be derived from FieldWidgetBase or FieldWidgetBufferBase. This property only applies to component fields.
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
        void patch(entt::registry& registry, entt::entity entity)
        {
            registry.patch<T>(entity);
        }

        template<typename T>
        T& copy(entt::registry& dstRegistry, entt::entity dstEntity, const entt::meta_any& comp)
        {
            return dstRegistry.emplace_or_replace<T>(dstEntity, comp.cast<T>());
        }
    }

}
