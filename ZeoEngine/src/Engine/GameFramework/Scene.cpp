#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	Scene::~Scene()
	{
		OnClenup();
	}

	void Scene::SetPath(const std::string& path)
	{
		m_Path = path;
		m_Name = GetNameFromPath(path);
	}

	Entity Scene::CreateEntity(const std::string& name, bool bIsInternal)
	{
		Entity entity = CreateEmptyEntity();

		auto entityCount = m_Entities.size();
		auto& coreComp = entity.AddComponent<CoreComponent>();
		{
			coreComp.Name = name;
			coreComp.CreationId = entityCount - 1;
			coreComp.bIsInternal = bIsInternal;
		}
		entity.AddComponent<TransformComponent>();
#if ENABLE_TEST
		entity.AddComponent<TestComponent>();
#endif
		// No need to sort on first entity
		if (entityCount > 1)
		{
			SortEntities();
		}

		return entity;
	}

	Entity Scene::CreateEmptyEntity()
	{
		Entity entity{ m_Registry.create(), this };
		// Insert entity Id to map
		m_Entities[entity];
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

	void Scene::OnUpdate(DeltaTime dt)
	{
		// Update scripts
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			// TODO: Move to Scene::OnBeginPlay
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}

			nsc.Instance->OnUpdate(dt);
		});
	}

	void Scene::OnRender()
	{
		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto cameraView = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : cameraView)
			{
				auto [transformComp, cameraComp] = cameraView.get<TransformComponent, CameraComponent>(entity);
				if (cameraComp.bIsPrimary)
				{
					mainCamera = &cameraComp.Camera;
					cameraTransform = transformComp.GetTransform();
					break;
				}
			}
		}
		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			{
				OnSceneRender();
			}
			Renderer2D::EndScene();
		}
	}

	void Scene::OnEvent(Event& e)
	{
		m_Registry.view<NativeScriptComponent>().each([&e](auto entity, auto& nsc)
		{
			if (nsc.Instance)
			{
				nsc.Instance->OnEvent(e);
			}
		});
	}

}
