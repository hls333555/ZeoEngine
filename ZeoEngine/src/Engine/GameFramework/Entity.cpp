#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

#include "Engine/Utils/ReflectionUtils.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {
	
	Entity::Entity(entt::entity handle, const Ref<Scene>& scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	bool Entity::IsValid() const
	{
		if (const auto scene = m_Scene.lock())
		{
			return scene->m_Registry.valid(*this);
		}
		return false;
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
		auto& transformComp = GetComponent<TransformComponent>();
		transformComp.Translation = translation;
		transformComp.Rotation = rotation;
		transformComp.Scale = scale;
		UpdateBounds();
	}

	const Vec3& Entity::GetTranslation() const
	{
		return GetComponent<TransformComponent>().Translation;
	}

	void Entity::SetTranslation(const Vec3& translation)
	{
		GetComponent<TransformComponent>().Translation = translation;
	}

	const Vec3& Entity::GetRotation() const
	{
		return GetComponent<TransformComponent>().Rotation;
	}

	void Entity::SetRotation(const Vec3& rotation)
	{
		GetComponent<TransformComponent>().Rotation = rotation;
	}

	const Vec3& Entity::GetScale() const
	{
		return GetComponent<TransformComponent>().Scale;
	}

	void Entity::SetScale(const Vec3& scale)
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
		for (const auto compId : GetOrderedComponentIds())
		{
			auto compInstance = GetComponentById(compId);
			auto* comp = compInstance.try_cast<IComponent>();
			if (comp->ComponentHelper)
			{
				boundingBox += comp->ComponentHelper->GetBounds().GetBox();
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

	entt::meta_any Entity::AddComponentById(entt::id_type compId, bool bIsDeserialize)
	{
		auto compType = entt::resolve(compId);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to add component with invalid component ID = {0}!", compId);
			return {};
		}

		if (HasComponentById(compId))
		{
			const char* compName = ReflectionUtils::GetMetaObjectName(compType);
			ZE_CORE_WARN("Failed to add {0} because current entity already contains the same component!", compName);
			return {};
		}
		auto compInstance = ReflectionUtils::ConstructComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
		auto* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		// Call this before OnComponentAdded so that newly added component can be queried within OnComponentAdded
		AddComponentId(compId);
		comp->CreateHelper(this);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentAdded(bIsDeserialize);
			// All bounds will be updated after deserialization
			if (!bIsDeserialize)
			{
				UpdateBounds();
			}
			ReflectionUtils::BindOnComponentDestroy(compType, m_Scene.lock()->m_Registry);
		}
		
		return compInstance;
	}

	void Entity::RemoveComponentById(entt::id_type compId)
	{
		auto compType = entt::resolve(compId);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to remove component with invalid component ID = {0}!", compId);
			return;
		}

		RemoveComponentId(compId);
		UpdateBounds();
		ReflectionUtils::RemoveComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
	}

	entt::meta_any Entity::GetComponentById(entt::id_type compId) const
	{
		auto compType = entt::resolve(compId);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to get component with invalid component ID = {0}!", compId);
			return {};
		}

		return ReflectionUtils::GetComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
	}

	bool Entity::HasComponentById(entt::id_type compId) const
	{
		auto compType = entt::resolve(compId);
		if (!compType) return false;

		auto bHas = ReflectionUtils::HasComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
		return bHas.cast<bool>();
	}

	entt::meta_any Entity::GetOrAddComponentById(entt::id_type compId, bool bIsDeserialize)
	{
		if (HasComponentById(compId))
		{
			return GetComponentById(compId);
		}
		else
		{
			return AddComponentById(compId, bIsDeserialize);
		}
	}

	void Entity::CopyAllComponents(Entity srcEntity, const std::vector<U32>& ignoredCompIds)
	{
		for (const auto compId : srcEntity.GetOrderedComponentIds())
		{
			if (std::find(ignoredCompIds.cbegin(), ignoredCompIds.cend(), compId) == ignoredCompIds.cend())
			{
				CopyComponentById(compId, srcEntity);
			}
		}
	}

	void Entity::CopyComponentById(entt::id_type compId, Entity srcEntity)
	{
		auto compType = entt::resolve(compId);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to copy component with invalid component ID = {0}!", compId);
			return;
		}

		auto srcCompInstance = srcEntity.GetComponentById(compId);
		auto* srcComp = srcCompInstance.try_cast<IComponent>();
		auto compInstance = ReflectionUtils::CopyComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle, srcCompInstance);
		auto* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->CreateHelper(this);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentCopied(srcComp);
			ReflectionUtils::BindOnComponentDestroy(compType, m_Scene.lock()->m_Registry);
		}
	}

	const std::vector<U32>& Entity::GetOrderedComponentIds() const
	{
		return GetComponent<CoreComponent>().OrderedComponents;
	}

	void Entity::AddComponentId(U32 Id)
	{
		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.push_back(Id);
	}

	void Entity::RemoveComponentId(U32 Id)
	{
		if (!HasComponent<CoreComponent>()) return;

		auto& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.erase(
			std::remove_if(coreComp.OrderedComponents.begin(), coreComp.OrderedComponents.end(),
				[Id](U32 componentId) { return componentId == Id; }),
			coreComp.OrderedComponents.end());
	}

}
