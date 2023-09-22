#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXScene.h"

namespace ZeoEngine {

	namespace Utils {

		static void ToLocalSpaceTransform(Entity entity)
		{
			const Entity parent = entity.GetParentEntity();
			if (!parent) return;

			const Mat4 parentTransform = parent.GetWorldTransform();
			const Mat4 localTransform = glm::inverse(parentTransform) * entity.GetTransform();
			Vec3 translation, rotation, scale;
			// NOTE: If we attach an entity with rotation to a parent entity with non-uniform scale, that child entity will become skewed
			// This is because parent scale is not applied in child local space and matrix decomposing will produce a shear matrix in addition to translation, rotation and scale
			// See TTransform<T>::GetRelativeTransform in Unreal Engine
			// https://gabormakesgames.com/blog_transforms_matrix_getters.html
			Math::DecomposeTransform(localTransform, translation, rotation, scale);
			entity.SetTransform(translation, rotation, scale);
		}

		static void ToWorldSpaceTransform(Entity entity)
		{
			const Entity parent = entity.GetParentEntity();
			if (!parent) return;

			const Mat4 worldTransform = entity.GetWorldTransform();
			Vec3 translation, rotation, scale;
			Math::DecomposeTransform(worldTransform, translation, rotation, scale);
			entity.SetTransform(translation, rotation, scale);
		}

	}

	Scene::Scene(SceneSpec spec)
		: m_Spec(spec)
		, m_ContextShared(CreateRef<SceneContext>())
	{
	}

	Scene::~Scene() = default;

	Ref<Scene> Scene::Copy()
	{
		auto newScene = CreateRef<Scene>(m_Spec);
		newScene->m_ContextShared = m_ContextShared;
		const auto coreView = GetComponentView<CoreComponent>();
		for (const auto e : coreView)
		{
			const Entity entity{ e, shared_from_this() };
			// Clone a new "empty" entity
			auto newEntity = newScene->CreateEntityWithUUID(entity.GetUUID(), entity.GetName());
			// Copy components to that entity
			newEntity.CopyAllRegisteredComponents(entity);
		}
		return newScene;
	}

	PhysXScene* Scene::CreatePhysicsScene()
	{
		return &AddContext<PhysXScene>(this);
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
		entity.AddComponent<RelationshipComponent>();
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

	// TODO: Rule: KeepRelative, KeepWorld, SnapTo
	void Scene::ParentEntity(Entity entity, Entity parent)
	{
		if (parent.IsDescendantOf(entity))
		{
			ZE_CORE_WARN("Parent entity {0} cannot become the child of its descendant entity {1}", entity.GetName(), parent.GetName());
			return;
		}

		if (Entity previousParent = entity.GetParentEntity())
		{
			UnparentEntity(entity);
		}

		auto& childComp = entity.GetComponent<RelationshipComponent>();
		childComp.ParentEntity = parent.GetUUID();
		auto& parentComp = parent.GetComponent<RelationshipComponent>();
		parentComp.ChildEntities.emplace_back(entity.GetUUID());
		childComp.HierarchyDepth = parentComp.HierarchyDepth + 1;

		auto& transformComp = entity.GetComponent<TransformComponent>();
		entity.AddComponent<WorldTransformComponent>(transformComp.Translation, transformComp.GetRotationInRadians(), transformComp.Scale);

		Utils::ToLocalSpaceTransform(entity);
	}

	// TODO: Rule: KeepRelative, KeepWorld
	void Scene::UnparentEntity(Entity entity)
	{
		const Entity parent = entity.GetParentEntity();
		if (!parent) return;

		auto& parentComp = parent.GetComponent<RelationshipComponent>();
		auto& parentChildren = parentComp.ChildEntities;
		parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), entity.GetUUID()), parentChildren.end());

		Utils::ToWorldSpaceTransform(entity);

		auto& childComp = entity.GetComponent<RelationshipComponent>();
		childComp.ParentEntity = 0;
		childComp.HierarchyDepth = 0;

		entity.RemoveComponent<WorldTransformComponent>();
	}

	// TODO: Should duplicate submesh entities
	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());
		const Mat4 worldTransform = entity.GetWorldTransform();
		newEntity.CopyAllRegisteredComponents(entity,
		{
			entt::type_hash<CoreComponent>::value(),
			entt::type_hash<IDComponent>::value(),
			entt::type_hash<RelationshipComponent>::value(),
			entt::type_hash<WorldTransformComponent>::value()
		});
		newEntity.SetTransform(worldTransform);
		return newEntity;
	}

	// TODO: Should destroy submesh entities
	void Scene::DestroyEntity(Entity entity)
	{
		auto* physicsScene = GetPhysicsScene();
		if (physicsScene)
		{
			if (entity.HasComponent<RigidBodyComponent>())
			{
				physicsScene->DestroyActor(entity);
			}

			if (entity.HasComponent<CharacterControllerComponent>())
			{
				physicsScene->DestroyCharacterController(entity);
			}
		}

		for (const UUID childID : entity.GetChildren())
		{
			UnparentEntity(GetEntityByUUID(childID));
		}

		const auto uuid = entity.GetUUID();
		destroy(entity);
		m_Entities.erase(uuid);
		SortEntities();
	}

	Entity Scene::GetEntityByUUID(UUID uuid) const
	{
		const auto it = m_Entities.find(uuid);
		if (it != m_Entities.end()) return it->second;
		return {};
	}

	Entity Scene::GetEntityByName(std::string_view name)
	{
		const auto coreView = GetComponentView<CoreComponent>();
		for (const auto e : coreView)
		{
			auto [coreComp] = coreView.get(e);
			if (coreComp.Name == name)
			{
				Entity entity{ e, shared_from_this() };
				return entity;
			}
		}
		return {};
	}

	Entity Scene::GetMainCameraEntity()
	{
		const auto cameraView = GetComponentView<CameraComponent>();
		for (const auto e : cameraView)
		{
			auto [cameraComp] = cameraView.get(e);
			if (cameraComp.bIsPrimary)
			{
				Entity entity{ e, shared_from_this() };
				return entity;
			}
		}
		return {};
	}

	void Scene::SortEntities()
	{
		// Sort entities by creation index
		// We assume that every entity has the CoreComponent which will never get removed
		sort<CoreComponent>([](const CoreComponent& lhs, const CoreComponent& rhs)
		{
			return lhs.EntityIndex < rhs.EntityIndex;
		});
	}

}
