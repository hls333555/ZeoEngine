#include "Scenes/MainEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	MainEditorScene::~MainEditorScene()
	{
		OnClenup();
	}

	void MainEditorScene::OnUpdate(DeltaTime dt)
	{
		// TODO:
		OnUpdateEditor(dt);
	}

	void MainEditorScene::OnRender(const EditorCamera& camera)
	{
		// TODO:
		OnRenderEditor(camera);
	}

	void MainEditorScene::OnEvent(Event& e)
	{
		// TODO:
		m_Registry.view<NativeScriptComponent>().each([&e](auto entity, auto& nsc)
		{
			if (nsc.Instance)
			{
				nsc.Instance->OnEvent(e);
			}
		});
	}

	void MainEditorScene::OnUpdateEditor(DeltaTime dt)
	{
		UpdateParticleSystem(dt);
	}

	void MainEditorScene::OnUpdateRuntime(DeltaTime dt)
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

		UpdateParticleSystem(dt);
	}

	void MainEditorScene::UpdateParticleSystem(DeltaTime dt)
	{
		m_Registry.view<ParticleSystemComponent>().each([dt](auto entity, auto& psc)
		{
			psc.UpdateParticleSystem(dt);
		});
	}

	void MainEditorScene::OnRenderEditor(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			RenderPrimitives();
		}
		Renderer2D::EndScene();
	}

	void MainEditorScene::OnRenderRuntime()
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
				RenderPrimitives();
			}
			Renderer2D::EndScene();
		}
	}

	void MainEditorScene::RenderPrimitives()
	{
		auto spriteGroup = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

		// Sort sprite entities
		spriteGroup.sort([&](const entt::entity lhs, const entt::entity rhs)
		{
			const auto& lsrc = spriteGroup.get<SpriteRendererComponent>(lhs);
			const auto& rsrc = spriteGroup.get<SpriteRendererComponent>(rhs);
			return lsrc.SortingOrder < rsrc.SortingOrder;
		});

		// Render sprites
		for (auto entity : spriteGroup)
		{
			auto [transformComp, spriteComp] = spriteGroup.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<int32_t>(entity));
		}

		// Render particle system
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& psc)
		{
			psc.RenderParticleSystem();
		});
	}

	void MainEditorScene::OnDeserialized()
	{
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& psc)
		{
			psc.CreateParticleSystem({});
		});
	}

	void MainEditorScene::OnClenup()
	{
		Scene::OnClenup();

		// Clear particle system reference for runtime on scene destruction
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& psc)
		{
			psc.RemoveCurrentParticleSystemInstance();
		});
	}

}
