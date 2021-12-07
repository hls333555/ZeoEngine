#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Entity Scene::CreateEntity(const std::string& name, const glm::vec3& translation)
	{
		return CreateEntityWithUUID(UUID(), name, translation);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const glm::vec3& translation)
	{
		Entity entity{ m_Registry.create(), this };

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

	SceneAsset::SceneAsset(const std::string& path)
		: AssetBase(path)
	{
	}

	AssetHandle<SceneAsset> SceneAsset::Create(const std::string& path)
	{
		// A way to allow std::make_shared() to access SceneAsset's private constructor
		class SceneAssetEnableShared : public SceneAsset
		{
		public:
			explicit SceneAssetEnableShared(const std::string& path)
				: SceneAsset(path) {}
		};

		return CreateRef<SceneAssetEnableShared>(path);
	}

	void SceneAsset::Reload()
	{
		Deserialize();
		m_Scene->PostLoad();
	}

	void SceneAsset::Serialize(const std::string& path)
	{
		if (path.empty()) return;

		if (path != GetPath())
		{
			SetPath(path);
		}

		ZE_CORE_ASSERT(m_Scene);
		SceneSerializer::Serialize(GetPath(), m_Scene);
	}

	void SceneAsset::Deserialize()
	{
		if (GetPath().empty()) return;

		ZE_CORE_ASSERT(m_Scene);
		SceneSerializer::Deserialize(GetPath(), m_Scene, this);
	}

}
