#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXScene.h"

namespace ZeoEngine {

	SceneSpec SceneSpec::Clone()
	{
		SceneSpec spec;
		spec.SceneObserverSystem = SceneObserverSystem->Clone();
		spec.bIsPhysicalScene = bIsPhysicalScene;
		return spec;
	}

	Scene::Scene(SceneSpec spec)
		: m_Spec(std::move(spec))
		, m_ContextShared(CreateRef<SceneContext>())
	{
		if (m_Spec.SceneObserverSystem)
		{
			m_Spec.SceneObserverSystem->SetScene(this);
			m_Spec.SceneObserverSystem->OnBind();
		}
	}

	Scene::~Scene()
	{
		if (m_Spec.SceneObserverSystem)
		{
			m_Spec.SceneObserverSystem->OnUnbind();
		}
	}

	void Scene::OnUpdate()
	{
		if (m_Spec.SceneObserverSystem)
		{
			m_Spec.SceneObserverSystem->OnUpdate(*this);
		}
	}

	Ref<Scene> Scene::Copy()
	{
		auto newScene = CreateRef<Scene>(m_Spec.Clone());
		newScene->m_ContextShared = m_ContextShared;
		ForEachComponentView<CoreComponent>([this, &newScene](auto entityID, auto& coreComp)
		{
			const Entity entity{ entityID, shared_from_this() };
			// Clone a new "empty" entity
			auto newEntity = newScene->CreateEntityWithUUID(entity.GetUUID(), entity.GetName());
			// Copy components to that entity
			newEntity.CopyAllRegisteredComponents(entity);
		});
		return newScene;
	}

	PhysXScene* Scene::CreatePhysicsScene()
	{
		if (m_Spec.bIsPhysicalScene)
		{
			return &AddContext<PhysXScene>();
		}
		return nullptr;
	}

	PhysXScene* Scene::GetPhysicsScene()
	{
		if (!HasContext<PhysXScene>()) return nullptr;
		return &GetContext<PhysXScene>();
	}

	void Scene::DestroyPhysicsScene()
	{
		if (HasContext<PhysXScene>())
		{
			RemoveContext<PhysXScene>();
		}
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
		auto* physicsScene = GetPhysicsScene();
		if (physicsScene)
		{
			if (entity.HasComponent<RigidBodyComponent>())
			{
				physicsScene->DestroyActor(entity);
			}
		}
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
