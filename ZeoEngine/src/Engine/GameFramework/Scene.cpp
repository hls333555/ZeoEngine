#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	Scene::~Scene()
	{
		for (const auto& system : m_Systems)
		{
			system->OnDestroy();
		}
	}

	void Scene::OnUpdate(DeltaTime dt) const
	{
		for (const auto& system : m_Systems)
		{
			system->OnUpdate(dt);
		}
	}

	Entity Scene::CreateEntity(const std::string& name, const glm::vec3& translation)
	{
		return CreateEntityWithUUID(UUID(), name, translation);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const glm::vec3& translation)
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
		m_Registry.destroy(entity);
		SortEntities();
	}

	Entity Scene::GetSelectedEntity() const
	{
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

	Ref<Level> Level::Create(std::string ID, const Ref<Scene>& scene)
	{
		auto level = CreateRef<Level>(std::move(ID), scene);
		level->Deserialize();
		scene->PostLoad();
		return level;
	}

	void Level::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));

		ZE_CORE_ASSERT(m_Scene);
		SceneSerializer::Serialize(GetID(), m_Scene);
	}

	void Level::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		ZE_CORE_ASSERT(m_Scene);
		SceneSerializer::Deserialize(GetID(), m_Scene);
	}

}
