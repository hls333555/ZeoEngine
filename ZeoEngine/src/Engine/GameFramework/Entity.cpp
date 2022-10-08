#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

#include "Engine/GameFramework/TypeRegistry.h"
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

	void Entity::SetTransform(const Vec3& translation, const Vec3& rotation, const Vec3& scale) const
	{
		auto& transformComp = GetComponent<TransformComponent>();
		transformComp.Translation = translation;
		transformComp.Rotation = glm::degrees(rotation);
		transformComp.Scale = scale;
		UpdateBounds();
	}

	const Vec3& Entity::GetTranslation() const
	{
		return GetComponent<TransformComponent>().Translation;
	}

	void Entity::SetTranslation(const Vec3& translation) const
	{
		GetComponent<TransformComponent>().Translation = translation;
	}

	Vec3 Entity::GetRotation() const
	{
		return GetComponent<TransformComponent>().GetRotationInRadians();
	}

	void Entity::SetRotation(const Vec3& rotation) const
	{
		GetComponent<TransformComponent>().Rotation = glm::degrees(rotation);
	}

	const Vec3& Entity::GetScale() const
	{
		return GetComponent<TransformComponent>().Scale;
	}

	void Entity::SetScale(const Vec3& scale) const
	{
		GetComponent<TransformComponent>().Scale = scale;
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

	const BoxSphereBounds& Entity::GetBounds() const
	{
		return GetComponent<BoundsComponent>().Bounds;
	}

	void Entity::UpdateBounds() const
	{
		if (!HasComponent<TransformComponent>()) return;

		Box boundingBox;
		// For each component, accumulate its bounding box
		for (const auto compID : GetOrderedComponentIDs())
		{
			if (auto* helper = ComponentHelperRegistry::GetComponentHelper(compID))
			{
				auto* comp = GetComponentByID(compID).try_cast<IComponent>();
				boundingBox += helper->GetBounds(comp).GetBox();
			}
		}
		// If we create a new empty entity, give it a default bounds
		GetComponent<BoundsComponent>().Bounds = boundingBox.bIsValid ? boundingBox : GetDefaultBounds();
	}

	BoxSphereBounds Entity::GetDefaultBounds() const
	{
		auto& transformComp = GetComponent<TransformComponent>();
		return { transformComp.Translation, { 1.0f, 1.0f, 1.0f }, 1.0f };
	}

	bool Entity::IsValid() const
	{
		if (const auto scene = m_Scene.lock())
		{
			return scene->m_Registry.valid(*this);
		}
		return false;
	}

	entt::meta_any Entity::AddComponentByID(U32 compID, bool bIsDeserialize)
	{
		auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to add component with invalid component ID = {0}!", compID);
			return {};
		}

		if (HasComponentByID(compID))
		{
			const char* compName = ReflectionUtils::GetMetaObjectName(compType);
			ZE_CORE_WARN("Failed to add {0} because current entity already contains the same component!", compName);
			return {};
		}
		auto compInstance = ReflectionUtils::ConstructComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
		auto* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->OwnerEntity = *this;
		// Call this before OnComponentAdded so that newly added component can be queried within OnComponentAdded
		AddComponentID(compID);
		if (auto* helper = ComponentHelperRegistry::GetComponentHelper(compID))
		{
			helper->OnComponentAdded(comp, bIsDeserialize);
			// All bounds will be updated after deserialization
			if (!bIsDeserialize)
			{
				UpdateBounds();
			}
			ReflectionUtils::BindOnComponentDestroy(compType, m_Scene.lock()->m_Registry);
		}
		
		return compInstance;
	}

	void Entity::RemoveComponentByID(U32 compID)
	{
		auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to remove component with invalid component ID = {0}!", compID);
			return;
		}

		RemoveComponentID(compID);
		UpdateBounds();
		ReflectionUtils::RemoveComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
	}

	entt::meta_any Entity::GetComponentByID(U32 compID) const
	{
		auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to get component with invalid component ID = {0}!", compID);
			return {};
		}

		return ReflectionUtils::GetComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
	}

	bool Entity::HasComponentByID(U32 compID) const
	{
		auto compType = entt::resolve(compID);
		if (!compType) return false;

		auto bHas = ReflectionUtils::HasComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
		return bHas.cast<bool>();
	}

	entt::meta_any Entity::GetOrAddComponentByID(U32 compID, bool bIsDeserialize)
	{
		if (HasComponentByID(compID))
		{
			return GetComponentByID(compID);
		}
		else
		{
			return AddComponentByID(compID, bIsDeserialize);
		}
	}

	void Entity::CopyAllComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIDs)
	{
		for (const auto compID : srcEntity.GetOrderedComponentIDs())
		{
			if (std::find(ignoredCompIDs.cbegin(), ignoredCompIDs.cend(), compID) == ignoredCompIDs.cend())
			{
				CopyComponentByID(compID, srcEntity);
			}
		}
	}

	void Entity::CopyComponentByID(U32 compID, Entity srcEntity)
	{
		auto compType = entt::resolve(compID);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to copy component with invalid component ID = {0}!", compID);
			return;
		}

		auto srcCompInstance = srcEntity.GetComponentByID(compID);
		auto* srcComp = srcCompInstance.try_cast<IComponent>();
		auto compInstance = ReflectionUtils::CopyComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle, srcCompInstance);
		if (auto* helper = ComponentHelperRegistry::GetComponentHelper(compID))
		{
			auto* comp = compInstance.try_cast<IComponent>();
			ZE_CORE_ASSERT(comp);
			helper->OnComponentCopied(comp, srcComp);
			ReflectionUtils::BindOnComponentDestroy(compType, m_Scene.lock()->m_Registry);
		}
	}

	const std::vector<U32>& Entity::GetOrderedComponentIDs() const
	{
		return GetComponent<CoreComponent>().OrderedComponents;
	}

	void Entity::AddComponentID(U32 id)
	{
		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.push_back(id);
	}

	void Entity::RemoveComponentID(U32 id)
	{
		if (!HasComponent<CoreComponent>()) return;

		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.erase(
			std::remove_if(coreComp.OrderedComponents.begin(), coreComp.OrderedComponents.end(),
				[id](U32 compID) { return compID == id; }),
			coreComp.OrderedComponents.end());
	}

}
