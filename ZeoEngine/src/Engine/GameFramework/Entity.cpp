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

	entt::meta_any Entity::AddTypeById(entt::id_type typeId)
	{
		auto type = entt::resolve_type(typeId);
		if (!type)
		{
			ZE_CORE_WARN("Failed to add component with invalid type ID = {0}!", typeId);
			return {};
		}

		if (HasTypeById(typeId))
		{
			auto typeName = GetMetaObjectDisplayName(type);
			ZE_CORE_WARN("Failed to add {0} because current entity already contains the same component!", *typeName);
			return {};
		}
		auto comp = type.construct(std::ref(m_Scene->m_Registry), m_EntityHandle);
		auto& baseComp = comp.cast<Component>();
		baseComp.OwnerEntity = *this;
		AddComponentId(typeId);
		BindOnDestroyFunc(type, m_Scene->m_Registry);
		return comp;
	}

	void Entity::RemoveTypeById(entt::id_type typeId)
	{
		auto type = entt::resolve_type(typeId);
		if (!type)
		{
			ZE_CORE_WARN("Failed to remove component with invalid type ID = {0}!", typeId);
			return;
		}

		RemoveComponentId(typeId);
		InternalRemoveType(type, m_Scene->m_Registry, m_EntityHandle);
	}

	entt::meta_any Entity::GetTypeById(entt::id_type typeId) const
	{
		auto type = entt::resolve_type(typeId);
		if (!type)
		{
			ZE_CORE_WARN("Failed to get component with invalid type ID = {0}!", typeId);
			return {};
		}

		return InternalGetType(type, m_Scene->m_Registry, m_EntityHandle);
	}

	bool Entity::HasTypeById(entt::id_type typeId) const
	{
		auto type = entt::resolve_type(typeId);
		if (!type) return false;

		auto res = InternalHasType(type, m_Scene->m_Registry, m_EntityHandle);
		return res.cast<bool>();
	}

	entt::meta_any Entity::GetOrAddTypeById(entt::id_type typeId)
	{
		if (HasTypeById(typeId))
		{
			return GetTypeById(typeId);
		}
		else
		{
			return AddTypeById(typeId);
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
