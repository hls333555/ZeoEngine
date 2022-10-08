#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Profile/Profiler.h"

namespace ZeoEngine {

	Scene::~Scene() = default;

	void Scene::OnUpdate(DeltaTime dt) const
	{
		ZE_PROFILE_FUNC();

		for (const auto& system : m_Systems)
		{
			system->OnUpdate(dt);
		}
	}

	void Scene::Copy(const Ref<Scene>& other)
	{
		m_Systems = other->m_Systems;
		m_Context = other->m_Context;
		other->ForEachComponentView<CoreComponent>([this, &other](auto entityID, auto& coreComp)
		{
			Entity entity{ entityID, other };
			// Clone a new "empty" entity
			auto newEntity = CreateEntityWithUUID(entity.GetUUID(), entity.GetName());
			// Copy components to that entity
			newEntity.CopyAllComponents(entity);
		});
		other->m_OnSceneCopiedDel.publish(shared_from_this());
	}

	Entity Scene::CreateEntity(const std::string& name, const Vec3& translation)
	{
		return CreateEntityWithUUID(UUID(), name, translation);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const Vec3& translation)
	{
		Entity entity{ m_Registry.create(), shared_from_this() };

		auto& coreComp = entity.AddComponent<CoreComponent>();
		{
			coreComp.Name = name;
			coreComp.EntityIndex = m_CurrentEntityIndex++;
		}
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<BoundsComponent>();
		entity.AddComponent<TransformComponent>(translation);

		// No need to sort if there is only one entity
		if (GetEntityCount() > 1)
		{
			SortEntities();
		}
		m_Entities[uuid] = entity;
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());
		// Copy all components but IDComponent(UUID)
		newEntity.CopyAllComponents(entity, { entt::type_hash<IDComponent>::value() });
		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		const auto uuid = entity.GetUUID();
		m_Registry.destroy(entity);
		m_Entities.erase(uuid);
		SortEntities();
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_Entities.find(uuid) != m_Entities.end())
		{
			return m_Entities[uuid];
		}
		return {};
	}

	void Scene::SortEntities()
	{
		// Sort entities by creation index
		// We assume that every entity has the CoreComponent which will never get removed
		m_Registry.sort<CoreComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.EntityIndex < rhs.EntityIndex;
		});
	}

}
