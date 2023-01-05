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

		template<U32 TagHash>
		void AddTag() const
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			if (HasTag<TagHash>()) return;

			m_Scene.lock()->emplace<entt::tag<TagHash>>(m_EntityHandle);
		}

		template<U32 TagHash>
		bool HasTag() const
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			return m_Scene.lock()->all_of<entt::tag<TagHash>>(m_EntityHandle);
		}

		template<U32 TagHash>
		void RemoveTag() const
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			m_Scene.lock()->erase<entt::tag<TagHash>>(m_EntityHandle);
		}

		template<U32 TagHash>
		auto RemoveTagIfExist() const
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			return m_Scene.lock()->remove<entt::tag<TagHash>>(m_EntityHandle);
		}

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
		void RemoveComponent()
		{
			ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
			ZE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			RemoveComponentID(entt::type_hash<T>::value());
			m_Scene.lock()->erase<T>(m_EntityHandle);
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
		Entity GetParentEntity() const;
		const std::vector<UUID>& GetChildren() const;
		bool HasAnyChildren() const;
		const std::string& GetName() const;

		Mat4 GetTransform() const;
		void SetTransform(const Mat4& transform);
		void SetTransform(const Vec3& translation, const Vec3& rotation);
		void SetTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale);
		const Vec3& GetTranslation() const;
		void SetTranslation(const Vec3& translation);
		Vec3 GetRotation() const;
		void SetRotation(const Vec3& rotation);
		const Vec3& GetScale() const;
		void SetScale(const Vec3& scale);

		Mat4 GetWorldTransform() const;
		void SetWorldTransform(const Mat4& transform);
		void SetWorldTransform(const Vec3& translation, const Vec3& rotation);
		void SetWorldTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale);
		const Vec3& GetWorldTranslation() const;
		void SetWorldTranslation(const Vec3& translation);
		Vec3 GetWorldRotation() const;
		void SetWorldRotation(const Vec3& rotation);
		const Vec3& GetWorldScale() const;
		void SetWorldScale(const Vec3& scale);

		Vec3 GetForwardVector() const;
		Vec3 GetRightVector() const;
		Vec3 GetUpVector() const;

		void UpdateBounds() const;
		const BoxSphereBounds& GetBounds() const;

		bool IsAncestorOf(Entity entity) const
		{
			const auto& children = GetChildren();

			if (children.empty()) return false;

			for (const UUID child : children)
			{
				if (child == entity.GetUUID()) return true;
			}

			for (const UUID child : children)
			{
				if (GetScene().GetEntityByUUID(child).IsAncestorOf(entity)) return true;
			}

			return false;
		}

		bool IsDescendantOf(Entity entity) const
		{
			return entity.IsAncestorOf(*this);
		}

		/** Returns true if entity is still alive. */
		bool IsValid() const;

		/** Returns true if entity is not a null entity. */
		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator U32() const { return static_cast<U32>(m_EntityHandle); }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene.lock() == other.m_Scene.lock(); }
		bool operator!=(const Entity& other) const { return !(*this == other); }

		entt::meta_any AddComponentByID(U32 compID, bool bIsDeserialize = false) const;
		void RemoveComponentByID(U32 compID) const;
		entt::meta_any GetComponentByID(U32 compID) const;
		bool HasComponentByID(U32 compID) const;
		entt::meta_any GetOrAddComponentByID(U32 compID, bool bIsDeserialize = false);
		void PatchComponentByID(U32 compID, U32 fieldID);
		void CopyAllRegisteredComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIDs = {}) const;
		void CopyComponentByID(U32 compID, Entity srcEntity) const;

		const std::vector<U32>& GetRegisteredComponentIDs() const;
		void AddComponentID(U32 compID) const;
		void RemoveComponentID(U32 compID) const;

		void AddBillboardTexture(const std::string& texturePath);
		void RemoveBillboardTexture(const std::string& texturePath);

		void MarkWorldTransformDirty() const;

	private:
		BoxSphereBounds GetDefaultBounds() const;

		void MarkTransformDirty() const;
		void MarkTransformDirtyRecursively(const Entity& entity) const;

	private:
		entt::entity m_EntityHandle{ entt::null };
		Weak<Scene> m_Scene; // We store scene as weak pointer here because we must know if scene is valid as entity's lifetime may pass its owned scene

	};

}
