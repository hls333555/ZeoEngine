#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name, bool bIsInternal)
	{
		Entity entity = CreateEmptyEntity();

		auto& coreComp = entity.AddComponent<CoreComponent>();
		{
			coreComp.Name = name;
			coreComp.CreationId = m_EntityCount++;
			coreComp.bIsInternal = bIsInternal;
		}
		entity.AddComponent<TransformComponent>();
#if ENABLE_TEST
		entity.AddComponent<TestComponent>();
#endif
		SortEntities(); // TODO: Particle View Camera will also go here

		return entity;
	}

	Entity Scene::CreateEmptyEntity()
	{
		return { m_Registry.create(), this };
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
		{
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
	}

	void Scene::OnRender()
	{
		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transformComp, cameraComp] = view.get<TransformComponent, CameraComponent>(entity);
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

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

			// Sort entities for rendering based on z position or creation order (if z positions are equivalent)
			group.sort([&](const entt::entity lhs, const entt::entity rhs)
			{
				const auto& ltc = group.get<TransformComponent>(lhs);
				const auto& rtc = group.get<TransformComponent>(rhs);
				if (ltc.Translation.z == rtc.Translation.z)
				{
					return m_Registry.get<CoreComponent>(lhs).CreationId < m_Registry.get<CoreComponent>(rhs).CreationId;
				}
				return ltc.Translation.z < rtc.Translation.z;
			});

			for (auto entity : group)
			{
				auto [transformComp, spriteComp] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				if (spriteComp.Texture)
				{
					Renderer2D::DrawRotatedQuad(transformComp.GetTransform(), spriteComp.Texture, spriteComp.TextureTiling, { 0.0f, 0.0f }, spriteComp.TintColor);
				}
				else
				{
					Renderer2D::DrawRotatedQuad(transformComp.GetTransform(), spriteComp.TintColor);
				}
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
