#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

#include "Engine/Core/ReflectionHelper.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {
	
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	bool Entity::IsValid() const
	{
		return m_Scene->m_Registry.valid(*this);
	}

	entt::meta_any Entity::AddComponentById(entt::id_type compId)
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
		auto compInstance = Reflection::ConstructComponent(compType, m_Scene->m_Registry, m_EntityHandle);
		IComponent* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->CreateHelper(this);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentAdded();
			Reflection::BindOnComponentDestroy(compType, m_Scene->m_Registry);
		}
		AddComponentId(compId);
		
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
		Reflection::RemoveComponent(compType, m_Scene->m_Registry, m_EntityHandle);
	}

	entt::meta_any Entity::GetComponentById(entt::id_type compId) const
	{
		auto compType = entt::resolve(compId);
		if (!compType)
		{
			ZE_CORE_WARN("Failed to get component with invalid component ID = {0}!", compId);
			return {};
		}

		return Reflection::GetComponent(compType, m_Scene->m_Registry, m_EntityHandle);
	}

	bool Entity::HasComponentById(entt::id_type compId) const
	{
		auto compType = entt::resolve(compId);
		if (!compType) return false;

		auto bHas = Reflection::HasComponent(compType, m_Scene->m_Registry, m_EntityHandle);
		return bHas.cast<bool>();
	}

	entt::meta_any Entity::GetOrAddComponentById(entt::id_type compId)
	{
		if (HasComponentById(compId))
		{
			return GetComponentById(compId);
		}
		else
		{
			return AddComponentById(compId);
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

		auto compInstance = Reflection::CopyComponent(compType, m_Scene->m_Registry, m_EntityHandle, srcEntity.GetComponentById(compId));
		IComponent* comp = compInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		comp->CreateHelper(this);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentCopied();
			Reflection::BindOnComponentDestroy(compType, m_Scene->m_Registry);
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
		CoreComponent& coreComp = GetComponent<CoreComponent>();
		coreComp.OrderedComponents.erase(
			std::remove_if(coreComp.OrderedComponents.begin(), coreComp.OrderedComponents.end(),
				[Id](uint32_t componentId) { return componentId == Id; }),
			coreComp.OrderedComponents.end());
	}

}
