#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

#include "Engine/Core/ReflectionHelper.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {
	
	Entity::Entity(entt::entity handle, const Ref<Scene>& scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	bool Entity::IsValid() const
	{
		return m_Scene.lock()->m_Registry.valid(*this);
	}

	void Entity::UpdateBounds()
	{
		if (!HasComponent<TransformComponent>()) return;

		Box boundingBox;
		// For each component, accumulate its bounding box
		for (const auto compId : GetOrderedComponentIds())
		{
			auto compInstance = GetComponentById(compId);
			IComponent* comp = compInstance.try_cast<IComponent>();
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
		return BoxSphereBounds(transformComp.Translation, { 1.0f, 1.0f, 1.0f }, 1.0f);
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
			auto compName = GetMetaObjectDisplayName(compType);
			ZE_CORE_WARN("Failed to add {0} because current entity already contains the same component!", *compName);
			return {};
		}
		auto compInstance = Reflection::ConstructComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
		IComponent* comp = compInstance.try_cast<IComponent>();
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
			Reflection::BindOnComponentDestroy(compType, m_Scene.lock()->m_Registry);
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
		Reflection::RemoveComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
	}

	entt::meta_any Entity::GetComponentById(entt::id_type compId) const
	{
		auto compType = entt::resolve(compId);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to get component with invalid component ID = {0}!", compId);
			return {};
		}

		return Reflection::GetComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
	}

	bool Entity::HasComponentById(entt::id_type compId) const
	{
		auto compType = entt::resolve(compId);
		if (!compType) return false;

		auto bHas = Reflection::HasComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle);
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

	void Entity::CopyAllComponents(Entity srcEntity, const std::vector<uint32_t>& ignoredCompIds)
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
		IComponent* srcComp = srcCompInstance.try_cast<IComponent>();
		auto compInstance = Reflection::CopyComponent(compType, m_Scene.lock()->m_Registry, m_EntityHandle, srcCompInstance);
		IComponent* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->CreateHelper(this);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentCopied(srcComp);
			Reflection::BindOnComponentDestroy(compType, m_Scene.lock()->m_Registry);
		}
	}

	const std::vector<glm::uint32_t>& Entity::GetOrderedComponentIds() const
	{
		return GetComponent<CoreComponent>().OrderedComponents;
	}

	void Entity::AddComponentId(uint32_t Id)
	{
		CoreComponent& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.push_back(Id);
	}

	void Entity::RemoveComponentId(uint32_t Id)
	{
		if (!HasComponent<CoreComponent>()) return;

		CoreComponent& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.erase(
			std::remove_if(coreComp.OrderedComponents.begin(), coreComp.OrderedComponents.end(),
				[Id](uint32_t componentId) { return componentId == Id; }),
			coreComp.OrderedComponents.end());
	}

}
