#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

#include "Engine/Utils/ReflectionUtils.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {
	
	Entity::Entity(entt::entity handle, const Ref<Scene>& scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	UUID Entity::GetUUID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	const std::string& Entity::GetName() const
	{
		return GetComponent<CoreComponent>().Name;
	}

	Mat4 Entity::GetTransform() const
	{
		return GetComponent<TransformComponent>().GetTransform();
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
		const auto& transformComp = GetComponent<TransformComponent>();
		return { transformComp.Translation, { 1.0f, 1.0f, 1.0f }, 1.0f };
	}

	const BoxSphereBounds& Entity::GetBounds() const
	{
		return GetComponent<BoundsComponent>().Bounds;
	}

	bool Entity::IsValid() const
	{
		return m_Scene.expired() ? false : m_Scene.lock()->valid(m_EntityHandle);
	}

	entt::meta_any Entity::AddComponentByID(U32 compID, bool bIsDeserialize) // TODO: bIsDeserialized
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

	void Entity::RemoveComponentByID(U32 compID)
	{
		ZE_CORE_ASSERT(IsValid(), "Entity is not valid!");
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

	void Entity::CopyAllRegisteredComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIDs)
	{
		for (const auto compID : srcEntity.GetRegisteredComponentIDs())
		{
			if (std::find(ignoredCompIDs.cbegin(), ignoredCompIDs.cend(), compID) == ignoredCompIDs.cend())
			{
				CopyComponentByID(compID, srcEntity);
			}
		}
	}

	void Entity::CopyComponentByID(U32 compID, Entity srcEntity)
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
	}

	const std::vector<U32>& Entity::GetRegisteredComponentIDs() const
	{
		return GetComponent<CoreComponent>().OrderedComponents;
	}

	void Entity::AddComponentID(U32 compID)
	{
		if (!ReflectionUtils::IsComponentRegistered(compID)) return;

		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.push_back(compID);
	}

	void Entity::RemoveComponentID(U32 compID)
	{
		if (!HasComponent<CoreComponent>()) return;

		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.erase(
			std::remove_if(coreComp.OrderedComponents.begin(), coreComp.OrderedComponents.end(),
				[compID](U32 inCompID) { return inCompID == compID; }),
			coreComp.OrderedComponents.end());
	}

}
