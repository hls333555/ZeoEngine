#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	ISystem::ISystem(Scene* scene)
		: m_Scene(scene)
	{
	}

	void RenderSystem::OnUpdate(DeltaTime dt)
	{
		ForEachView<ParticleSystemComponent>([dt](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		});

		ForEachView<ParticleSystemPreviewComponent>([dt](auto entity, auto& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnUpdate(dt);
		});
	}

	void RenderSystem::OnRenderEditor(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			OnRender();
		}
		Renderer2D::EndScene();
	}

	void RenderSystem::OnRenderRuntime()
	{
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto cameraView = m_Scene->m_Registry.view<TransformComponent, CameraComponent>();
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
				OnRender();
			}
			Renderer2D::EndScene();
		}
	}

	void RenderSystem::OnRender()
	{
		auto spriteGroup = m_Scene->m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

		// Sort sprite entities
		spriteGroup.sort([&](const entt::entity lhs, const entt::entity rhs)
		{
			const auto& lSpriteComp = spriteGroup.get<SpriteRendererComponent>(lhs);
			const auto& rSpriteComp = spriteGroup.get<SpriteRendererComponent>(rhs);
			return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
		});

		// Render sprites
		for (auto entity : spriteGroup)
		{
			auto [transformComp, spriteComp] = spriteGroup.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<int32_t>(entity));
		}

		ForEachView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
		});

		ForEachView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnRender();
		});
	}

	static void RemoveParticleSystemInstance(ParticleSystemComponent& particleComp)
	{
		if (particleComp.Template)
		{
			particleComp.Template->RemoveParticleSystemInstance(particleComp.Instance);
		}
	}

	void RenderSystem::OnDestroy()
	{
		ForEachView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			RemoveParticleSystemInstance(particleComp);
		});

		ForEachView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			RemoveParticleSystemInstance(particlePreviewComp);
		});
	}

	void NativeScriptSystem::OnUpdate(DeltaTime dt)
	{
		ForEachView<NativeScriptComponent>([=](auto entity, auto& nativeScriptComp)
		{
			// TODO: Move to OnBeginPlay
			if (!nativeScriptComp.Instance)
			{
				nativeScriptComp.Instance = nativeScriptComp.InstantiateScript();
				nativeScriptComp.Instance->m_Entity = Entity{ entity, m_Scene };
				nativeScriptComp.Instance->OnCreate();
			}

			nativeScriptComp.Instance->OnUpdate(dt);
		});
	}

	void NativeScriptSystem::OnEvent(Event& e)
	{
		ForEachView<NativeScriptComponent>([&e](auto entity, auto& nativeScriptComp)
		{
			if (nativeScriptComp.Instance)
			{
				nativeScriptComp.Instance->OnEvent(e);
			}
		});
	}

}
