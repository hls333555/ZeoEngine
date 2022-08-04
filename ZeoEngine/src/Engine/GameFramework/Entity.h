#pragma once

#include <glm/glm.hpp>

#include "Engine/GameFramework/Scene.h"
#include "Engine/Core/ReflectionCore.h"
#include "Engine/Math/BoxSphereBounds.h"

namespace ZeoEngine {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, const Ref<Scene>& scene);
		Entity(const Entity&) = default;

		bool IsValid() const;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ZE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& comp = m_Scene.lock()->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			// Call this before OnComponentAdded so that newly added component can be queried within OnComponentAdded
			AddComponentId(entt::type_hash<T>::value());
			comp.CreateHelper(this);
			if (comp.ComponentHelper)
			{
				comp.ComponentHelper->OnComponentAdded(false);
				UpdateBounds();
				m_Scene.lock()->m_Registry.on_destroy<T>().template connect<&Reflection::on_destroy<T>>();
			}
			return comp;
		}

		template<typename T>
		auto RemoveComponent()
		{
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			RemoveComponentId(entt::type_hash<T>::value());
			UpdateBounds();
			return m_Scene.lock()->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		auto RemoveComponentIfExist()
		{
			if (HasComponent<T>())
			{
				RemoveComponentId(entt::type_hash<T>::value());
				UpdateBounds();
				return m_Scene.lock()->m_Registry.remove<T>(m_EntityHandle);
			}
			return static_cast<SizeT>(0);
		}

		template<typename T>
		T& GetComponent() const
		{
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene.lock()->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const
		{
			return m_Scene.lock()->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T, typename... Func>
		void PatchComponent(Func&&... func)
		{
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene.lock()->m_Registry.patch<T>(m_EntityHandle, std::forward<Func>(func)...);
		}

		UUID GetUUID() const;
		const std::string& GetName() const;
		Mat4 GetTransform() const;
		void SetTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale);
		const Vec3& GetTranslation() const;
		void SetTranslation(const Vec3& translation);
		const Vec3& GetRotation() const;
		void SetRotation(const Vec3& rotation);
		const Vec3& GetScale() const;
		void SetScale(const Vec3& scale);
		Vec3 GetForwardVector() const;
		Vec3 GetRightVector() const;
		Vec3 GetUpVector() const;
		const BoxSphereBounds& GetBounds() const;
		void UpdateBounds() const;
		BoxSphereBounds GetDefaultBounds() const;

		Ref<Scene> GetScene() const { return m_Scene.lock(); }

		// TODO:
		/** Returns the entity identifier without the version. */
		U32 GetEntityId() const { return entt::to_entity(m_EntityHandle); }

		/** This only checks if this entity is a null-entity while IsValid() checks if this entity is still alive. */
		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator U32() const { return static_cast<U32>(m_EntityHandle); }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene.lock() == other.m_Scene.lock(); }
		bool operator!=(const Entity& other) const { return !(*this == other); }

	public:
		entt::meta_any AddComponentById(entt::id_type compId, bool bIsDeserialize = false);
		void RemoveComponentById(entt::id_type compId);
		entt::meta_any GetComponentById(entt::id_type compId) const;
		bool HasComponentById(entt::id_type compId) const;
		entt::meta_any GetOrAddComponentById(entt::id_type compId, bool bIsDeserialize = false);
		void CopyAllComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIds = {});
		void CopyComponentById(entt::id_type compId, Entity srcEntity);

		const std::vector<U32>& GetOrderedComponentIds() const;
		void AddComponentId(U32 Id);
		void RemoveComponentId(U32 Id);

	private:
		entt::entity m_EntityHandle{ entt::null };
		Weak<Scene> m_Scene; // Weak reference which will not prevent scene from being destroyed (e.g. when end play, copied scene should be destroyed)

	};

}
