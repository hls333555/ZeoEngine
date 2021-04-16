#pragma once

#include <glm/glm.hpp>

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity&) = default;

		bool IsValid() const;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ZE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& comp = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			comp.OwnerEntity = *this;
			if (comp.ComponentHelper)
			{
				comp.ComponentHelper->SetOwnerEntity(*this);
				m_Scene->m_Registry.on_destroy<T>().template connect<&IComponentHelper::OnComponentDestroy>(comp.ComponentHelper);
			}
			AddComponentId(entt::type_hash<T>::value());
			return comp;
		}

		template<typename T>
		void RemoveComponent()
		{
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			RemoveComponentId(entt::type_hash<T>::value());
			return m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetComponent() const
		{
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T, typename... Func>
		void PatchComponent(Func&&... func)
		{
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.patch<T>(m_EntityHandle, std::forward<Func>(func)...);
		}

		std::string GetEntityName() const;
		glm::mat4 GetEntityTransform() const;
		glm::vec3 GetEntityTranslation() const;
		glm::vec3 GetEntityRotation() const;
		glm::vec3 GetEntityScale() const;

		/** Returns the entity identifier without the version. */
		uint32_t GetEntityId() const { return static_cast<uint32_t>(entt::registry::entity(m_EntityHandle)); }

		/** This only checks if this entity is a null-entity while IsValid() checks if this entity is still alive. */
		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }

	public:
		entt::meta_any AddComponentById(entt::id_type compId);
		void RemoveComponentById(entt::id_type compId);
		entt::meta_any GetComponentById(entt::id_type compId) const;
		bool HasComponentById(entt::id_type compId) const;
		entt::meta_any GetOrAddComponentById(entt::id_type compId);

		const std::vector<uint32_t>& GetOrderedComponentIds() const;
		void AddComponentId(uint32_t Id);
		void RemoveComponentId(uint32_t Id);

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

	};

}
