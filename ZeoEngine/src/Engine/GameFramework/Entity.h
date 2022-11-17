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

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<IComponent, T>, "T must be derived from 'IComponent'!");

			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			ZE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& comp = m_Scene.lock()->emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			comp.OwnerEntity = *this;
			const U32 compID = entt::type_hash<T>::value();
			AddComponentID(compID);
			return comp;
		}

		template<typename T, typename... Args>
		T& GetOrAddComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<IComponent, T>, "T must be derived from 'IComponent'!");

			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			if (HasComponent<T>())
			{
				return GetComponent<T>();
			}
			return AddComponent<T>(std::forward<Args>(args)...);
		}

		template<typename T>
		auto RemoveComponent()
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			RemoveComponentID(entt::type_hash<T>::value());
			return m_Scene.lock()->erase<T>(m_EntityHandle);
		}

		template<typename T>
		auto RemoveComponentIfExist()
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			RemoveComponentID(entt::type_hash<T>::value());
			return m_Scene.lock()->remove<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetComponent() const
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene.lock()->get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			return m_Scene.lock()->all_of<T>(m_EntityHandle);
		}

		template<typename T, typename... Func>
		void PatchComponent(Func&&... func)
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene.lock()->patch<T>(m_EntityHandle, std::forward<Func>(func)...);
		}

		template<typename T, typename... Func>
		void PatchComponentSingleField(U32 fieldID, Func&&... func)
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			auto& changeComp = AddComponent<struct FieldChangeComponent>();
			changeComp.FieldID = fieldID;
			m_Scene.lock()->patch<T>(m_EntityHandle, std::forward<Func>(func)...);
			RemoveComponent<FieldChangeComponent>();
		}

		Scene& GetScene() const { return *m_Scene.lock(); }

		UUID GetUUID() const;
		const std::string& GetName() const;
		Mat4 GetTransform() const;
		void SetTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale);
		const Vec3& GetTranslation() const;
		void SetTranslation(const Vec3& translation);
		Vec3 GetRotation() const;
		void SetRotation(const Vec3& rotation);
		const Vec3& GetScale() const;
		void SetScale(const Vec3& scale);
		Vec3 GetForwardVector() const;
		Vec3 GetRightVector() const;
		Vec3 GetUpVector() const;

		void UpdateBounds() const;
		const BoxSphereBounds& GetBounds() const;

		/** Returns true if entity is still alive. */
		bool IsValid() const;

		/** Returns true if entity is not a null entity. */
		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator U32() const { return static_cast<U32>(m_EntityHandle); }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene.lock() == other.m_Scene.lock(); }
		bool operator!=(const Entity& other) const { return !(*this == other); }

		entt::meta_any AddComponentByID(U32 compID, bool bIsDeserialize = false);
		void RemoveComponentByID(U32 compID);
		entt::meta_any GetComponentByID(U32 compID) const;
		bool HasComponentByID(U32 compID) const;
		entt::meta_any GetOrAddComponentByID(U32 compID, bool bIsDeserialize = false);
		void PatchComponentByID(U32 compID, U32 fieldID);
		void CopyAllRegisteredComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIDs = {});
		void CopyComponentByID(U32 compID, Entity srcEntity);

		const std::vector<U32>& GetRegisteredComponentIDs() const;
		void AddComponentID(U32 compID);
		void RemoveComponentID(U32 compID);

	private:
		BoxSphereBounds GetDefaultBounds() const;

	private:
		entt::entity m_EntityHandle{ entt::null };
		Weak<Scene> m_Scene; // We store scene as weak pointer here because we must know if scene is valid as entity's lifetime may pass its owned scene

	};

}
