#include "ZEpch.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {
	
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	bool Entity::IsValid() const
	{
		return m_Scene->m_Registry.valid(*this);
	}

	entt::meta_any Entity::AddType(entt::meta_type type, entt::registry& registry)
	{
		AddComponentId(type.type_id());
		return type.construct(std::ref(registry), m_EntityHandle);
	}

	entt::meta_any Entity::AddTypeById(entt::id_type typeId, entt::registry& registry)
	{
		AddComponentId(typeId);
		return entt::resolve_type(typeId).construct(std::ref(registry), m_EntityHandle);
	}

	void Entity::RemoveType(entt::meta_type type, entt::registry& registry)
	{
		RemoveComponentId(type.type_id());
		type.func("remove"_hs).invoke({}, std::ref(registry), m_EntityHandle);
	}

	void Entity::AddComponentId(uint32_t Id)
	{
		m_Scene->m_Entities[m_EntityHandle].push_back(Id);
	}

	void Entity::RemoveComponentId(uint32_t Id)
	{
		m_Scene->m_Entities[m_EntityHandle].erase(
			std::remove_if(m_Scene->m_Entities[m_EntityHandle].begin(), m_Scene->m_Entities[m_EntityHandle].end(),
				[Id](uint32_t componentId) { return componentId == Id; }),
			m_Scene->m_Entities[m_EntityHandle].end());
	}

}
