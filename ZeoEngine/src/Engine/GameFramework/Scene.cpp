#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

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
		// Sort entities by creation order
		// We assume that every entity has the CoreComponent which will never get removed
		m_Registry.sort<CoreComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.CreationId < rhs.CreationId;
		});
	}

	SceneAsset::SceneAsset(const std::string& path)
		: AssetBase(path)
	{
	}

	Ref<SceneAsset> SceneAsset::Create(const std::string& path)
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
		SceneSerializer::Deserialize(GetPath(), m_Scene);
	}

}
