#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void Scene::SetPath(const std::string& path)
	{
		m_Path = path;
		m_Name = GetNameFromPath(path);
	}

	Entity Scene::CreateEntity(const std::string& name, bool bIsInternal)
	{
		Entity entity = CreateEmptyEntity();

		auto& coreComp = entity.AddComponent<CoreComponent>();
		{
			coreComp.Name = name;
			coreComp.CreationId = m_EntityCount - 1;
			coreComp.bIsInternal = bIsInternal;
		}
		entity.AddComponent<TransformComponent>();
#if ENABLE_TEST
		entity.AddComponent<TestComponent>();
#endif
		// No need to sort on first entity
		if (m_EntityCount > 1)
		{
			SortEntities();
		}

		return entity;
	}

	Entity Scene::CreateEmptyEntity()
	{
		Entity entity{ m_Registry.create(), this };
		++m_EntityCount;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
		SortEntities();
	}

	void Scene::SortEntities()
	{
		ZE_CORE_TRACE("Sorting entities");

		// Sort entities by creation order
		// We assume that every entity has the CoreComponent which will never get removed
		m_Registry.sort<CoreComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.CreationId < rhs.CreationId;
		});
	}

}
