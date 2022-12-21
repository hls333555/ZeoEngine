#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

#include "Engine/Utils/ReflectionUtils.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Tags.h"

namespace ZeoEngine {
	
	Entity::Entity(entt::entity handle, const Ref<Scene>& scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	UUID Entity::GetUUID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	Entity Entity::GetParentEntity() const
	{
		const UUID parentID = GetComponent<RelationshipComponent>().ParentEntity;
		return GetScene().GetEntityByUUID(parentID);
	}

	const std::vector<UUID>& Entity::GetChildren() const
	{
		return GetComponent<RelationshipComponent>().ChildEntities;
	}

	bool Entity::HasAnyChildren() const
	{
		return !GetComponent<RelationshipComponent>().ChildEntities.empty();
	}

	const std::string& Entity::GetName() const
	{
		return GetComponent<CoreComponent>().Name;
	}

	Mat4 Entity::GetTransform() const
	{
		return GetComponent<TransformComponent>().GetTransform();
	}

	void Entity::SetTransform(const Mat4& transform)
	{
		PatchComponent<TransformComponent>([&transform](TransformComponent& transformComp)
		{
			transformComp.SetTransform(transform);
		});
	}

	void Entity::SetTransform(const Vec3& translation, const Vec3& rotation)
	{
		PatchComponent<TransformComponent>([&translation, &rotation](TransformComponent& transformComp)
		{
			transformComp.Translation = translation;
			transformComp.Rotation = glm::degrees(rotation);
		});
	}

	void Entity::SetTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale)
	{
		PatchComponent<TransformComponent>([&translation, &rotation, &scale](TransformComponent& transformComp)
		{
			transformComp.Translation = translation;
			transformComp.Rotation = glm::degrees(rotation);
			transformComp.Scale = scale;
		});
	}

	const Vec3& Entity::GetTranslation() const
	{
		return GetComponent<TransformComponent>().Translation;
	}

	void Entity::SetTranslation(const Vec3& translation)
	{
		PatchComponent<TransformComponent>([&translation](TransformComponent& transformComp)
		{
			transformComp.Translation = translation;
		});
	}

	Vec3 Entity::GetRotation() const
	{
		return GetComponent<TransformComponent>().GetRotationInRadians();
	}

	void Entity::SetRotation(const Vec3& rotation)
	{
		PatchComponent<TransformComponent>([&rotation](TransformComponent& transformComp)
		{
			transformComp.Rotation = glm::degrees(rotation);
		});
	}

	const Vec3& Entity::GetScale() const
	{
		return GetComponent<TransformComponent>().Scale;
	}

	void Entity::SetScale(const Vec3& scale)
	{
		PatchComponent<TransformComponent>([&scale](TransformComponent& transformComp)
		{
			transformComp.Scale = scale;
		});
	}

	Mat4 Entity::GetWorldTransform() const
	{
		if (HasComponent<WorldTransformComponent>())
		{
			return GetComponent<WorldTransformComponent>().GetTransform();
		}

		return GetTransform();
	}

	void Entity::SetWorldTransform(const Mat4& transform)
	{
		if (HasComponent<WorldTransformComponent>())
		{
			PatchComponent<WorldTransformComponent>([&transform](WorldTransformComponent& worldTransformComp)
			{
				worldTransformComp.SetTransform(transform);
			});
			MarkTransformDirty();
		}
		else
		{
			SetTransform(transform);
		}
	}

	void Entity::SetWorldTransform(const Vec3& translation, const Vec3& rotation)
	{
		if (HasComponent<WorldTransformComponent>())
		{
			PatchComponent<WorldTransformComponent>([&translation, &rotation](WorldTransformComponent& worldTransformComp)
			{
				worldTransformComp.Translation = translation;
				worldTransformComp.Rotation = rotation;
			});
			MarkTransformDirty();
		}
		else
		{
			SetTransform(translation, rotation);
		}
	}

	void Entity::SetWorldTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale)
	{
		if (HasComponent<WorldTransformComponent>())
		{
			PatchComponent<WorldTransformComponent>([&translation, &rotation, &scale](WorldTransformComponent& worldTransformComp)
			{
				worldTransformComp.Translation = translation;
				worldTransformComp.Rotation = rotation;
				worldTransformComp.Scale = scale;
			});
			MarkTransformDirty();
		}
		else
		{
			SetTransform(translation, rotation, scale);
		}
	}

	const Vec3& Entity::GetWorldTranslation() const
	{
		if (HasComponent<WorldTransformComponent>())
		{
			return GetComponent<WorldTransformComponent>().Translation;
		}

		return GetTranslation();
	}

	void Entity::SetWorldTranslation(const Vec3& translation)
	{
		if (HasComponent<WorldTransformComponent>())
		{
			PatchComponent<WorldTransformComponent>([&translation](WorldTransformComponent& worldTransformComp)
			{
				worldTransformComp.Translation = translation;
			});
			MarkTransformDirty();
		}
		else
		{
			SetTranslation(translation);
		}
	}

	Vec3 Entity::GetWorldRotation() const
	{
		if (HasComponent<WorldTransformComponent>())
		{
			return GetComponent<WorldTransformComponent>().Rotation;
		}

		return GetRotation();
	}

	void Entity::SetWorldRotation(const Vec3& rotation)
	{
		if (HasComponent<WorldTransformComponent>())
		{
			PatchComponent<WorldTransformComponent>([&rotation](WorldTransformComponent& worldTransformComp)
			{
				worldTransformComp.Rotation = rotation;
			});
			MarkTransformDirty();
		}
		else
		{
			SetRotation(rotation);
		}
	}

	const Vec3& Entity::GetWorldScale() const
	{
		if (HasComponent<WorldTransformComponent>())
		{
			return GetComponent<WorldTransformComponent>().Scale;
		}

		return GetScale();
	}

	void Entity::SetWorldScale(const Vec3& scale)
	{
		if (HasComponent<WorldTransformComponent>())
		{
			PatchComponent<WorldTransformComponent>([&scale](WorldTransformComponent& worldTransformComp)
			{
				worldTransformComp.Scale = scale;
			});
			MarkTransformDirty();
		}
		else
		{
			SetScale(scale);
		}
	}

	Vec3 Entity::GetForwardVector() const
	{
		return Math::GetForwardVector(GetRotation());
	}

	Vec3 Entity::GetRightVector() const
	{
		return Math::GetRightVector(GetRotation());
	}

	Vec3 Entity::GetUpVector() const
	{
		return Math::GetUpVector(GetRotation());
	}

	void Entity::UpdateBounds() const
	{
		auto& boundsComp = GetComponent<BoundsComponent>();
		Box boundingBox;
		for (const auto& [compID, func] : boundsComp.BoundsCalculationFuncs)
		{
			boundingBox += func(*this).GetBox();
		}
		boundsComp.Bounds = boundingBox.bIsValid ? boundingBox : GetDefaultBounds();
	}

	BoxSphereBounds Entity::GetDefaultBounds() const
	{
		return { GetWorldTranslation(), { 1.0f, 1.0f, 1.0f }, 1.0f };
	}

	// See TransformSystem::OnUpdate
	void Entity::MarkWorldTransformDirty() const
	{
		AddTag<Tag::AnyTransformDirty>();
		RemoveTagIfExist<Tag::LocalTransformDirty>();
		MarkTransformDirtyRecursively(*this);
	}

	void Entity::MarkTransformDirty() const
	{
		AddTag<Tag::AnyTransformDirty>();
		AddTag<Tag::LocalTransformDirty>();
		MarkTransformDirtyRecursively(*this);
	}

	void Entity::MarkTransformDirtyRecursively(const Entity& entity) const
	{
		for (const UUID childID : entity.GetChildren())
		{
			Entity child = GetScene().GetEntityByUUID(childID);
			child.AddTag<Tag::AnyTransformDirty>();

			MarkTransformDirtyRecursively(child);
		}
	}

	const BoxSphereBounds& Entity::GetBounds() const
	{
		return GetComponent<BoundsComponent>().Bounds;
	}

	bool Entity::IsValid() const
	{
		return m_Scene.expired() ? false : m_Scene.lock()->valid(m_EntityHandle);
	}

	entt::meta_any Entity::AddComponentByID(U32 compID, bool bIsDeserialize) const // TODO: bIsDeserialized
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
		const auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to add component with unregistered component ID = {0}!", compID);
			return {};
		}

		if (HasComponentByID(compID))
		{
			const char* compName = ReflectionUtils::GetMetaObjectName(compType);
			ZE_CORE_WARN("Failed to add {0} because current entity already contains the same component!", compName);
			return {};
		}
		auto compInstance = ReflectionUtils::ConstructComponent(compType, *m_Scene.lock(), m_EntityHandle);
		auto* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->OwnerEntity = *this;
		AddComponentID(compID);
		
		return compInstance;
	}

	void Entity::RemoveComponentByID(U32 compID) const
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
		ZE_CORE_ASSERT(HasComponentByID(compID), "Entity does not have component!");
		const auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to remove component with unregistered component ID = {0}!", compID);
			return;
		}

		RemoveComponentID(compID);
		ReflectionUtils::RemoveComponent(compType, *m_Scene.lock(), m_EntityHandle);
	}

	entt::meta_any Entity::GetComponentByID(U32 compID) const
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
		ZE_CORE_ASSERT(HasComponentByID(compID), "Entity does not have component!");
		const auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to get component with unregistered component ID = {0}!", compID);
			return {};
		}

		return ReflectionUtils::GetComponent(compType, *m_Scene.lock(), m_EntityHandle);
	}

	bool Entity::HasComponentByID(U32 compID) const
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
		const auto compType = entt::resolve(compID);
		if (!compType) return false;

		auto bHas = ReflectionUtils::HasComponent(compType, *m_Scene.lock(), m_EntityHandle);
		return bHas.cast<bool>();
	}

	entt::meta_any Entity::GetOrAddComponentByID(U32 compID, bool bIsDeserialize)
	{
		if (HasComponentByID(compID))
		{
			return GetComponentByID(compID);
		}
		return AddComponentByID(compID, bIsDeserialize);
	}

	void Entity::PatchComponentByID(U32 compID, U32 fieldID)
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
		const auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to patch component with unregistered component ID = {0}!", compID);
			return;
		}

		auto& changeComp = AddComponent<FieldChangeComponent>();
		changeComp.FieldID = fieldID;
		ReflectionUtils::PatchComponent(compType, *m_Scene.lock(), m_EntityHandle);
		RemoveComponent<FieldChangeComponent>();
	}

	void Entity::CopyAllRegisteredComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIDs) const
	{
		for (const auto compID : srcEntity.GetRegisteredComponentIDs())
		{
			if (std::find(ignoredCompIDs.cbegin(), ignoredCompIDs.cend(), compID) == ignoredCompIDs.cend())
			{
				CopyComponentByID(compID, srcEntity);
			}
		}
	}

	void Entity::CopyComponentByID(U32 compID, Entity srcEntity) const
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
		const auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to copy component with unregistered component ID = {0}!", compID);
			return;
		}

		auto srcCompInstance = srcEntity.GetComponentByID(compID);
		auto compInstance = ReflectionUtils::CopyComponent(compType, *m_Scene.lock(), m_EntityHandle, srcCompInstance);
		auto* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->OwnerEntity = *this;
		AddComponentID(compID);
	}

	const std::vector<U32>& Entity::GetRegisteredComponentIDs() const
	{
		return GetComponent<CoreComponent>().OrderedComponents;
	}

	void Entity::AddComponentID(U32 compID) const
	{
		if (!ReflectionUtils::IsComponentRegistered(compID)) return;

		auto& coreComp = GetComponent<CoreComponent>();
		auto& registeredComps = coreComp.OrderedComponents;
		if (std::find(registeredComps.begin(), registeredComps.end(), compID) == registeredComps.end())
		{
			registeredComps.push_back(compID);
		}
	}

	void Entity::RemoveComponentID(U32 compID) const
	{
		if (!HasComponent<CoreComponent>()) return;

		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.erase(
			std::remove_if(coreComp.OrderedComponents.begin(), coreComp.OrderedComponents.end(),
				[compID](U32 inCompID) { return inCompID == compID; }),
			coreComp.OrderedComponents.end());
	}

}
