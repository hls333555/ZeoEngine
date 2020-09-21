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

	Entity Scene::CreateEntity(const std::string& name, bool bForInternalUse)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>();
		auto& tagComp = entity.AddComponent<TagComponent>();
		tagComp.Tag = name.empty() ? "Entity" : name;
		tagComp.bForInternalUse = bForInternalUse;

		return entity;
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
		glm::mat4* cameraTransform = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transformComp, cameraComp] = view.get<TransformComponent, CameraComponent>(entity);
				if (cameraComp.bIsPrimary)
				{
					mainCamera = &cameraComp.Camera;
					cameraTransform = &transformComp.Transform;
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
				auto [transformComp, spriteComp] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				if (spriteComp.Texture)
				{
					Renderer2D::DrawRotatedQuad(transformComp, spriteComp.Texture, spriteComp.TextureTiling, { 0.0f, 0.0f }, spriteComp.TintColor);
				}
				else
				{
					Renderer2D::DrawRotatedQuad(transformComp, spriteComp.TintColor);
				}
			}

			Renderer2D::EndScene();
		}
	}

}
