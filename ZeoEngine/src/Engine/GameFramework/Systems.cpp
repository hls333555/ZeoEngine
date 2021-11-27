#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/GameFramework/ScriptableEntity.h"

namespace ZeoEngine {

	ISystem::ISystem(Scene* scene)
		: m_Scene(scene)
	{
	}

	void RenderSystem::OnUpdate(DeltaTime dt)
	{
		ForEachComponentView<ParticleSystemComponent>([dt](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		});
	}

	void RenderSystem::OnRenderEditor(const EditorCamera& camera)
	{
		Renderer::BeginScene(camera, true);
		{
			OnRender();
		}
		Renderer::EndScene();
	}

	void RenderSystem::OnRenderRuntime()
	{
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		ForEachComponentView<TransformComponent, CameraComponent>([&mainCamera, &cameraTransform](auto entity, auto& transformComp, auto& cameraComp)
		{
			if (cameraComp.bIsPrimary)
			{
				mainCamera = &cameraComp.Camera;
				cameraTransform = transformComp.GetTransform();
				return;
			}
		});
		if (mainCamera)
		{
			Renderer::BeginScene(*mainCamera, cameraTransform);
			{
				OnRender();
			}
			Renderer::EndScene();
		}
	}

	void RenderSystem::OnRender()
	{
		// Setup lights
		ForEachComponentView<TransformComponent, LightComponent, BillboardComponent>([this](auto entity, auto& transformComp, auto& lightComp, auto& billboardComp)
		{
			switch (lightComp.Type)
			{
				case LightComponent::LightType::DirectionalLight:
				{
					const auto& directionalLight = lightComp.GetLight<DirectionalLight>();
					Renderer::SetupDirectionalLight(transformComp.Rotation, directionalLight);

					// Draw arrow visualizer when selected
					if (m_Scene->GetSelectedEntity() == entity)
					{
						const glm::vec3 forward = glm::rotate(glm::quat(transformComp.Rotation), { 0.0f, 0.0f, -1.0f });
						const glm::vec3 up = glm::rotate(glm::quat(transformComp.Rotation), { 0.0f, 1.0f, 0.0f });
						const glm::vec3 right = glm::rotate(glm::quat(transformComp.Rotation), { 1.0f, 0.0f, 0.0f });
						const glm::vec3 endPosition = transformComp.Translation + glm::normalize(forward) * 1.0f;

						const glm::mat4 rotationMatrixUp = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), up);
						const glm::vec3 directionUp = rotationMatrixUp * glm::vec4(-forward, 1.0f);
						const glm::vec3 endPositionUp = endPosition + glm::normalize(directionUp) * 0.2f;

						const glm::mat4 rotationMatrixDown = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), up);
						const glm::vec3 directionDown = rotationMatrixDown * glm::vec4(-forward, 1.0f);
						const glm::vec3 endPositionDown = endPosition + glm::normalize(directionDown) * 0.2f;

						const glm::mat4 rotationMatrixRight = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), right);
						const glm::vec3 directionRight = rotationMatrixRight * glm::vec4(-forward, 1.0f);
						const glm::vec3 endPositionRight = endPosition + glm::normalize(directionRight) * 0.2f;

						const glm::mat4 rotationMatrixLeft = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), right);
						const glm::vec3 directionLeft = rotationMatrixLeft * glm::vec4(-forward, 1.0f);
						const glm::vec3 endPositionLeft = endPosition + glm::normalize(directionLeft) * 0.2f;

						const glm::vec4 color = { 0.5f, 0.5f, 0.5f, 1.0f };
						Renderer::DrawLine(transformComp.Translation, endPosition, color);
						Renderer::DrawLine(endPosition, endPositionUp, color);
						Renderer::DrawLine(endPosition, endPositionDown, color);
						Renderer::DrawLine(endPosition, endPositionRight, color);
						Renderer::DrawLine(endPosition, endPositionLeft, color);
					}
					break;
				}
				case LightComponent::LightType::PointLight:
				{
					const auto& pointLight = lightComp.GetLight<PointLight>();
					Renderer::AddPointLight(transformComp.Translation, pointLight);

					// Draw sphere visualizer when selected
					if (m_Scene->GetSelectedEntity() == entity)
					{
						const glm::mat4 translation = glm::translate(glm::mat4(1.0f), transformComp.Translation);
						float radius = pointLight->GetRadius() * 2.0f;
						const glm::mat4 scale = glm::scale(glm::mat4(1.0f), { radius, radius, 1.0f });
						const glm::mat4 transformXY = translation * scale;
						const glm::mat4 transformXZ = translation * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), { 1.0f, 0.0f, 0.0f }) * scale;
						const glm::mat4 transformYZ = translation * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), { 0.0f, 1.0f, 0.0f }) * scale;
						const glm::vec4 color = { 0.5f, 0.5f, 0.5f, 1.0f };
						Renderer::DrawCircle(transformXY, color);
						Renderer::DrawCircle(transformXZ, color);
						Renderer::DrawCircle(transformYZ, color);
					}
					break;
				}
				case LightComponent::LightType::SpotLight:
				{
					break;
				}
				default:
					break;
			}
		});
		Renderer::UploadLightData();

		// Render meshes
		ForEachComponentGroup<TransformComponent>(entt::get<MeshRendererComponent>, [](auto entity, auto& transformComp, auto& meshComp)
		{
			if (meshComp.Mesh)
			{
				Renderer::DrawMesh(transformComp.GetTransform(), meshComp.Mesh->GetMesh(), static_cast<int32_t>(entity));
			}
		});

		// Render billboards
		ForEachComponentView<TransformComponent, BillboardComponent>([this](auto e, auto& transformComp, auto& billboardComp)
		{
			if (billboardComp.Texture)
			{
				Entity entity = { e, m_Scene };
				const glm::vec4 tintColor = entity.HasComponent<LightComponent>() ? entity.GetComponent<LightComponent>().GetColor() : glm::vec4(1.0f);
				Renderer::DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.Texture->GetTexture(), { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<int32_t>(e));
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
		ForEachComponentView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			RemoveParticleSystemInstance(particleComp);
		});
	}

	void RenderSystem2D::OnRenderEditor(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			OnRender();
		}
		Renderer2D::EndScene();
	}

	void RenderSystem2D::OnRenderRuntime()
	{
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		ForEachComponentView<TransformComponent, CameraComponent>([&mainCamera, &cameraTransform](auto entity, auto& transformComp, auto& cameraComp)
		{
			if (cameraComp.bIsPrimary)
			{
				mainCamera = &cameraComp.Camera;
				cameraTransform = transformComp.GetTransform();
				return;
			}
		});
		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			{
				OnRender();
			}
			Renderer2D::EndScene();
		}
	}

	void RenderSystem2D::OnRender()
	{
		// Render sprites
		ForEachComponentGroup<TransformComponent>(entt::get<SpriteRendererComponent>, [](auto entity, auto& transformComp, auto& spriteComp)
		{
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<int32_t>(entity));
		},
		[](std::tuple<TransformComponent&, SpriteRendererComponent&> lhs, std::tuple<TransformComponent&, SpriteRendererComponent&> rhs)
		{
			const auto& lSpriteComp = std::get<1>(lhs);
			const auto& rSpriteComp = std::get<1>(rhs);
			return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
		});

		// Render circles
		ForEachComponentView<TransformComponent, CircleRendererComponent>([](auto entity, auto& transformComp, auto& circleComp)
		{
			Renderer2D::DrawCircle(transformComp.GetTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<int32_t>(entity));
		});

		// Render particle systems
		ForEachComponentView<ParticleSystemComponent>([](auto entity, auto& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
		});
	}

	void ParticlePreviewRenderSystem::OnUpdate(DeltaTime dt)
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([dt](auto entity, auto& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnUpdate(dt);
		});
	}

	void ParticlePreviewRenderSystem::OnRender(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
			{
				particlePreviewComp.Instance->OnRender();
			});
		}
		Renderer2D::EndScene();
	}

	void ParticlePreviewRenderSystem::OnDestroy()
	{
		ForEachComponentView<ParticleSystemPreviewComponent>([](auto entity, auto& particlePreviewComp)
		{
			RemoveParticleSystemInstance(particlePreviewComp);
		});
	}

	void MaterialPreviewRenderSystem::OnRender(const EditorCamera& camera)
	{
		Renderer::BeginScene(camera);
		{
			ForEachComponentView<TransformComponent, LightComponent>([](auto entity, auto& transformComp, auto& lightComp)
			{
				Renderer::SetupDirectionalLight(transformComp.Rotation, lightComp.GetLight<DirectionalLight>());
			});
			ForEachComponentGroup<TransformComponent>(entt::get<MeshRendererComponent, MaterialPreviewComponent>, [](auto entity, auto& transformComp, auto& meshComp, auto& materialPreviewComp)
			{
				const auto& mesh = meshComp.Mesh->GetMesh();
				mesh->SetMaterial(0, materialPreviewComp.Template);
				Renderer::DrawMesh(transformComp.GetTransform(), mesh);
			});
		}
		Renderer::EndScene();
	}

	void NativeScriptSystem::OnUpdate(DeltaTime dt)
	{
		ForEachComponentView<NativeScriptComponent>([=](auto entity, auto& nativeScriptComp)
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
		ForEachComponentView<NativeScriptComponent>([&e](auto entity, auto& nativeScriptComp)
		{
			if (nativeScriptComp.Instance)
			{
				nativeScriptComp.Instance->OnEvent(e);
			}
		});
	}

	void PhysicsSystem::OnUpdate(DeltaTime dt)
	{

	}


	void PhysicsSystem::OnRuntimeStart()
	{

	}


	void PhysicsSystem::OnRuntimeStop()
	{

	}

	void PhysicsSystem2D::OnUpdate(DeltaTime dt)
	{
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

		ForEachComponentView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
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

	void PhysicsSystem2D::OnRuntimeStart()
	{
		const b2Vec2 gravity = { 0.0f, -9.8f };
		m_PhysicsWorld = new b2World(gravity);
		ForEachComponentView<Rigidbody2DComponent>([this](auto e, auto& rb2dComp)
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

	void PhysicsSystem2D::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

}
