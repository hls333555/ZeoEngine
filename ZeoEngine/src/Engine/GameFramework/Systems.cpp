#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/GameFramework/ScriptableEntity.h"

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
		// Render sprites
		{
			auto spriteGroup = m_Scene->m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

			// Sort sprite entities
			spriteGroup.sort([&](const entt::entity lhs, const entt::entity rhs)
			{
				const auto& lSpriteComp = spriteGroup.get<SpriteRendererComponent>(lhs);
				const auto& rSpriteComp = spriteGroup.get<SpriteRendererComponent>(rhs);
				return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
			});

			for (auto entity : spriteGroup)
			{
				auto [transformComp, spriteComp] = spriteGroup.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<int32_t>(entity));
			}
		}

		// Render circles
		ForEachView<TransformComponent, CircleRendererComponent>([](auto entity, auto& transformComp, auto& circleComp)
		{
			Renderer2D::DrawCircle(transformComp.GetTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<int32_t>(entity));
		});

		ForEachView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
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
	}

	void ParticlePreviewRenderSystem::OnUpdate(DeltaTime dt)
	{
		ForEachView<ParticleSystemPreviewComponent>([dt](auto entity, auto& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnUpdate(dt);
		});
	}

	void ParticlePreviewRenderSystem::OnRender(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			ForEachView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
			{
				particlePreviewComp.Instance->OnRender();
			});
		}
		Renderer2D::EndScene();
	}

	void ParticlePreviewRenderSystem::OnDestroy()
	{
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

	void PhysicsSystem::OnUpdate(DeltaTime dt)
	{
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

		ForEachView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
		{
			Entity entity = { e, m_Scene };
			auto& transformComp = entity.GetComponent<TransformComponent>();

			// Retrieve transfrom from Box2D
			b2Body* body = static_cast<b2Body*>(rb2dComp.RuntimeBody);
			const auto& position = body->GetPosition();
			transformComp.Translation.x = position.x;
			transformComp.Translation.y = position.y;
			transformComp.Rotation.z = body->GetAngle();
		});
	}

	static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:	return b2_staticBody;
			case Rigidbody2DComponent::BodyType::Dynamic:	return b2_dynamicBody;
			case Rigidbody2DComponent::BodyType::Kinematic:	return b2_kinematicBody;
		}

		ZE_CORE_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

	void PhysicsSystem::OnRuntimeStart()
	{
		const b2Vec2 gravity = { 0.0f, -9.8f };
		m_PhysicsWorld = new b2World(gravity);
		ForEachView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
		{
			Entity entity = { e, m_Scene };
			auto& transformComp = entity.GetComponent<TransformComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2dComp.Type);
			bodyDef.position.Set(transformComp.Translation.x, transformComp.Translation.y);
			bodyDef.angle = transformComp.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2dComp.bFixedRotation);
			rb2dComp.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2dComp = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2dComp.Size.x * transformComp.Scale.x, bc2dComp.Size.y * transformComp.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2dComp.Density;
				fixtureDef.friction = bc2dComp.Friction;
				fixtureDef.restitution = bc2dComp.Restitution;
				fixtureDef.restitutionThreshold = bc2dComp.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2dComp = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2dComp.Offset.x, cc2dComp.Offset.y);
				circleShape.m_radius = cc2dComp.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2dComp.Density;
				fixtureDef.friction = cc2dComp.Friction;
				fixtureDef.restitution = cc2dComp.Restitution;
				fixtureDef.restitutionThreshold = cc2dComp.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		});
	}

	void PhysicsSystem::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

}
