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

	entt::meta_any Entity::AddTypeById(entt::id_type typeId)
	{
		AddComponentId(typeId);
		return entt::resolve_type(typeId).construct(std::ref(m_Scene->m_Registry), m_EntityHandle);
	}

	void Entity::RemoveTypeById(entt::id_type typeId)
	{
		RemoveComponentId(typeId);
		entt::resolve_type(typeId).func("remove"_hs).invoke({}, std::ref(m_Scene->m_Registry), m_EntityHandle);
	}

	entt::meta_any Entity::GetTypeById(entt::id_type typeId) const
	{
		return entt::resolve_type(typeId).func("get"_hs).invoke({}, std::ref(m_Scene->m_Registry), m_EntityHandle);
	}

	entt::meta_any Entity::HasTypeById(entt::id_type typeId) const
	{
		return entt::resolve_type(typeId).func("has"_hs).invoke({}, std::ref(m_Scene->m_Registry), m_EntityHandle);
	}

	entt::meta_any Entity::GetOrAddTypeById(entt::id_type typeId)
	{
		return entt::resolve_type(typeId).func("get_or_emplace"_hs).invoke({}, std::ref(m_Scene->m_Registry), m_EntityHandle);
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
