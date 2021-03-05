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

	std::string Entity::GetEntityName() const
	{
		return GetComponent<CoreComponent>().Name;
	}

	glm::mat4 Entity::GetEntityTransform() const
	{
		return GetComponent<TransformComponent>().GetTransform();
	}

	glm::vec3 Entity::GetEntityTranslation() const
	{
		return GetComponent<TransformComponent>().Translation;
	}

	glm::vec3 Entity::GetEntityRotation() const
	{
		return GetComponent<TransformComponent>().Rotation;
	}

	glm::vec3 Entity::GetEntityScale() const
	{
		return GetComponent<TransformComponent>().Scale;
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
		auto compInstance = compType.construct(std::ref(m_Scene->m_Registry), m_EntityHandle);
		Component* comp = compInstance.try_cast<Component>();
		ZE_CORE_ASSERT(comp);
		comp->OwnerEntity = *this;
		AddComponentId(compId);
		Reflection::BindOnDestroy(compType, m_Scene->m_Registry);
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
