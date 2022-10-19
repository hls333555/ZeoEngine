#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	Scene::Scene(Scope<SceneObserverSystemBase> sceneObserverSystem)
		: m_SceneObserverSystem(std::move(sceneObserverSystem))
	{
		m_SceneObserverSystem->SetScene(this);
		m_SceneObserverSystem->OnBind();
	}

	Scene::~Scene()
	{
		m_SceneObserverSystem->OnUnbind();
	}

	void Scene::OnUpdate()
	{
		m_SceneObserverSystem->OnUpdate(*this);
	}

	void Scene::Copy(const Ref<Scene>& other)
	{
		m_Context = other->m_Context;
		other->ForEachComponentView<CoreComponent>([this, &other](auto entityID, auto& coreComp)
		{
			const Entity entity{ entityID, other };
			// Clone a new "empty" entity
			auto newEntity = CreateEntityWithUUID(entity.GetUUID(), entity.GetName());
			// Copy components to that entity
			newEntity.CopyAllRegisteredComponents(entity);
		});
	}

	Entity Scene::CreateEntity(const std::string& name, const Vec3& translation)
	{
		return CreateEntityWithUUID(UUID(), name, translation);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const Vec3& translation)
	{
		Entity entity{ create(), shared_from_this() };

		auto& coreComp = entity.AddComponent<CoreComponent>();
		{
			coreComp.Name = name;
			coreComp.EntityIndex = m_CurrentEntityIndex++;
		}
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>(translation);
		entity.AddComponent<BoundsComponent>();

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
		newEntity.CopyAllRegisteredComponents(entity, { entt::type_hash<IDComponent>::value() });
		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		const auto uuid = entity.GetUUID();
		destroy(entity);
		m_Entities.erase(uuid);
		SortEntities();
	}

	Entity Scene::GetEntityByUUID(UUID uuid) const
	{
		if (m_Entities.find(uuid) != m_Entities.end())
		{
			return m_Entities.at(uuid);
		}
		return {};
	}

	Entity Scene::GetEntityByName(std::string_view name)
	{
		Entity entity;
		ForEachComponentView<CoreComponent>([&](auto e, auto& coreComp)
		{
			if (coreComp.Name == name)
			{
				entity = { e, shared_from_this() };
			}
		});
		return entity;
	}

	Entity Scene::GetMainCameraEntity()
	{
		Entity entity;
		ForEachComponentView<CameraComponent>([&](auto e, auto& cameraComp)
		{
			if (cameraComp.bIsPrimary)
			{
				entity = { e, shared_from_this() };
			}
		});
		return entity;
	}

	void Scene::SortEntities()
	{
		// Sort entities by creation index
		// We assume that every entity has the CoreComponent which will never get removed
		sort<CoreComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.EntityIndex < rhs.EntityIndex;
		});
	}

}
