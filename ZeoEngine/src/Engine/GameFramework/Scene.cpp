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

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>();
		auto& tagComp = entity.AddComponent<TagComponent>();
		tagComp.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::OnUpdate(DeltaTime dt)
	{
		// Update scripts
		{
			m_Registry.view<NativeScriptCompont>().each([=](auto entity, auto& nativeScriptComponent)
			{
				// TODO: Move to Scene::OnBeginPlay
				if (!nativeScriptComponent.Instance)
				{
					nativeScriptComponent.Instance = nativeScriptComponent.InstantiateScript();
					nativeScriptComponent.Instance->m_Entity = Entity{ entity, this };
					nativeScriptComponent.Instance->OnCreate();
				}

				nativeScriptComponent.Instance->OnUpdate(dt);
			});
		}

		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<CameraComponent>);
			for (auto entity : group)
			{
				auto [transformComponent, cameraComponent] = group.get<TransformComponent, CameraComponent>(entity);
				if (cameraComponent.bIsPrimary)
				{
					mainCamera = &cameraComponent.Camera;
					cameraTransform = &transformComponent.Transform;
					break;
				}
			}
		}
		if (mainCamera)
		{
			Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				if (spriteComponent.Texture)
				{
					Renderer2D::DrawRotatedQuad(transformComponent, spriteComponent.Texture, spriteComponent.TextureTiling, { 0.0f, 0.0f }, spriteComponent.TintColor);
				}
				else
				{
					Renderer2D::DrawRotatedQuad(transformComponent, spriteComponent.TintColor);
				}
			}

			Renderer2D::EndScene();
		}
		
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.bFixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

}
