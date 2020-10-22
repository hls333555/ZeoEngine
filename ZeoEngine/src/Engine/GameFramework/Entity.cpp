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

}
