#include "ZEpch.h"
#include "Engine/GameFramework/Systems.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "Engine/Core/Console.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/GameFramework/World.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Physics/PhysXCharacterController.h"
#include "Engine/Physics/PhysXScene.h"
#include "Engine/Physics/PhysXShapes.h"
#include "Engine/Renderer/RenderPass.h"

namespace ZeoEngine {

	ISystem::ISystem(WorldBase* world)
		: m_World(world)
	{
	}

	void SystemBase::OnCreate()
	{
		GetWorld()->m_OnActiveSceneChanged.connect<&SystemBase::OnActiveSceneChanged>(this);
	}

	void SystemBase::OnUpdateEditor(DeltaTime dt)
	{
		OnUpdateComponentObservers();
	}

	void SystemBase::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateComponentObservers();
	}

	void SystemBase::OnDestroy()
	{
		GetWorld()->m_OnActiveSceneChanged.disconnect(this);

		for (const auto& binder : m_ComponentEventBinders)
		{
			binder->OnUnbind(GetScene().get());
		}

		for (const auto& observer : m_ComponentObservers)
		{
			observer->OnUnbind();
		}
	}

	void SystemBase::OnActiveSceneChanged(Scene* scene, Scene* lastScene) const
	{
		for (const auto& binder : m_ComponentEventBinders)
		{
			binder->OnUnbind(lastScene);
			binder->OnBind(scene);
		}

		for (const auto& observer : m_ComponentObservers)
		{
			observer->OnUnbind();
			observer->OnBind(*scene);
		}
	}

	void SystemBase::OnUpdateComponentObservers() const
	{
		ZE_PROFILE_FUNC("Systems::OnUpdateComponentObservers");

		for (const auto& observer : m_ComponentObservers)
		{
			observer->OnUpdate(*GetScene());
		}
	}

	static void OnCameraObserverUpdate(Scene& scene, Entity entity)
	{
		auto& cameraComp = entity.GetComponent<CameraComponent>();
		cameraComp.Camera.RecalculateProjection();
	}

	void CameraSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<CameraComponent, &CameraSystem::OnCameraComponentAdded, CameraSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<CameraComponent, &CameraSystem::OnCameraComponentDestroy, CameraSystem>>(this);
		RegisterComponentObserver(&OnCameraObserverUpdate, entt::collector.update<CameraComponent>());
	}

	void CameraSystem::OnCameraComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/Camera.png.zasset");
	}

	void CameraSystem::OnCameraComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
	}

	void MeshSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<MeshRendererComponent, &MeshSystem::OnMeshRendererComponentAdded, MeshSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<MeshRendererComponent, &MeshSystem::OnMeshRendererComponentUpdated, MeshSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<MeshRendererComponent, &MeshSystem::OnMeshRendererComponentDestroy, MeshSystem>>(this);
	}

	void MeshSystem::OnMeshRendererComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		const auto& meshComp = entity.GetComponent<MeshRendererComponent>();
		// Invoke update callback for ctor with MeshAsset provided but not for copy ctor
		if (meshComp.MeshAsset && !meshComp.Instance)
		{
			entity.PatchComponentSingleField<MeshRendererComponent>("MeshAsset"_hs);
		}
		auto& boundsComp = entity.GetComponent<BoundsComponent>();
		boundsComp.BoundsCalculationFuncs[entt::type_hash<MeshRendererComponent>::value()] = [](Entity entity)
		{
			const auto& meshComp = entity.GetComponent<MeshRendererComponent>();
			const auto mesh = AssetLibrary::LoadAsset<Mesh>(meshComp.MeshAsset);
			return mesh ? mesh->GetBounds().TransformBy(entity.GetWorldTransform()) : BoxSphereBounds{};
		};
	}

	void MeshSystem::OnMeshRendererComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& meshComp = entity.GetComponent<MeshRendererComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "MeshAsset"_hs)
		{
			auto& meshInstance = meshComp.Instance;
			const auto mesh = AssetLibrary::LoadAsset<Mesh>(meshComp.MeshAsset);
			if (mesh)
			{
				// Copy default materials
				// For deserialization phase, materials will be deserialized and overwritten later
				meshComp.MaterialAssets = mesh->GetDefaultMaterialAssets();
				meshInstance = mesh->CreateInstance(scene);
			}
			else
			{
				meshInstance = nullptr;
				meshComp.MaterialAssets.clear();
			}
			// Update bounds immediately so that focusing after loading should work properly
			entity.UpdateBounds();
		}
	}

	void MeshSystem::OnMeshRendererComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<MeshRendererComponent>::value());
			});
		}
	}

	void MeshPreviewSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<MeshDetailComponent, &MeshPreviewSystem::OnMeshDetailComponentAdded, MeshPreviewSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<MeshDetailComponent, &MeshPreviewSystem::OnMeshDetailComponentUpdated, MeshPreviewSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<MeshDetailComponent, &MeshPreviewSystem::OnMeshDetailComponentDestroy, MeshPreviewSystem>>(this);
	}

	void MeshPreviewSystem::OnMeshDetailComponentAdded(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& boundsComp = entity.GetComponent<BoundsComponent>();
		boundsComp.BoundsCalculationFuncs[entt::type_hash<MeshDetailComponent>::value()] = [](Entity entity)
		{
			const auto& meshComp = entity.GetComponent<MeshDetailComponent>();
			const auto& mesh = meshComp.LoadedMesh;
			return mesh ? mesh->GetBounds().TransformBy(entity.GetWorldTransform()) : BoxSphereBounds{};
		};
	}

	void MeshPreviewSystem::OnMeshDetailComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& meshComp = entity.GetComponent<MeshDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "LoadedMesh"_hs)
		{
			meshComp.Instance = meshComp.LoadedMesh->CreateInstance(scene);
			// Update bounds immediately so that focusing after loading should work properly
			entity.UpdateBounds();
		}
	}

	void MeshPreviewSystem::OnMeshDetailComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<MeshDetailComponent>::value());
			});
		}
	}

	void MaterialPreviewSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<MaterialDetailComponent, &MaterialPreviewSystem::OnMaterialDetailComponentUpdated, MaterialPreviewSystem>>(this);
	}

	void MaterialPreviewSystem::OnMaterialDetailComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& materialComp = entity.GetComponent<MaterialDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShaderAsset"_hs)
		{
			materialComp.LoadedMaterial->NotifyShaderAssetChange();
		}
	}

	void TexturePreviewSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<TextureDetailComponent, &TexturePreviewSystem::OnTextureDetailComponentUpdated, TexturePreviewSystem>>(this);
	}

	void TexturePreviewSystem::OnTextureDetailComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& textureComp = entity.GetComponent<TextureDetailComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "SRGB"_hs || changeComp.FieldID == "GenerateMipmaps"_hs)
		{
			textureComp.LoadedTexture->Invalidate();
		}
		else if (changeComp.FieldID == "SamplerType"_hs)
		{
			textureComp.LoadedTexture->ChangeSampler(textureComp.SamplerType);
		}
	}

	void DirectionalLightSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<DirectionalLightComponent, &DirectionalLightSystem::OnDirectionalLightComponentAdded, DirectionalLightSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<DirectionalLightComponent, &DirectionalLightSystem::OnDirectionalLightComponentUpdated, DirectionalLightSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<DirectionalLightComponent, &DirectionalLightSystem::OnDirectionalLightComponentDestroy, DirectionalLightSystem>>(this);
	}

	void DirectionalLightSystem::OnDirectionalLightComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/DirectionalLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<DirectionalLightComponent>::value()] = [](Entity entity)
			{
				const Mat4 worldTransform = entity.GetWorldTransform();
				const Sphere sphere{ Math::GetTranslationFromTransform(worldTransform), 0.0f };
				return sphere;
			};
		});
	}

	void DirectionalLightSystem::OnDirectionalLightComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		auto& lightComp = entity.GetComponent<DirectionalLightComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShadowType"_hs)
		{
			const auto* shadowPass = EngineUtils::GetRenderPassFromContext<ScreenSpaceShadowPass>(scene.GetContextShared(), "ScreenSpaceShadow");
			const auto shadowShader = shadowPass->GetShadowShader();
			shadowShader->SetShaderVariantByMacro("SHADOW_TYPE", static_cast<U32>(lightComp.ShadowType));
		}
	}

	void DirectionalLightSystem::OnDirectionalLightComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<DirectionalLightComponent>::value());
			});
		}
	}

	void PointLightSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<PointLightComponent, &PointLightSystem::OnPointLightComponentAdded, PointLightSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<PointLightComponent, &PointLightSystem::OnPointLightComponentUpdated, PointLightSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<PointLightComponent, &PointLightSystem::OnPointLightComponentDestroy, PointLightSystem>>(this);
	}

	void PointLightSystem::OnPointLightComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/PointLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<PointLightComponent>::value()] = [](Entity entity)
			{
				const Mat4 worldTransform = entity.GetWorldTransform();
				const auto& lightComp = entity.GetComponent<PointLightComponent>();
				const Sphere sphere{ Math::GetTranslationFromTransform(worldTransform), lightComp.Range };
				return sphere;
			};
		});
	}

	void PointLightSystem::OnPointLightComponentUpdated(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& lightComp = entity.GetComponent<PointLightComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShadowType"_hs)
		{
			// TODO:
		}
		else if (changeComp.FieldID == "Range"_hs)
		{
			entity.PatchComponent<BoundsComponent>();
		}
	}

	void PointLightSystem::OnPointLightComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<PointLightComponent>::value());
			});
		}
	}

	void SpotLightSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentAddedBinder<SpotLightComponent, &SpotLightSystem::OnSpotLightComponentAdded, SpotLightSystem>>(this);
		RegisterComponentEventBinder<ComponentUpdatedBinder<SpotLightComponent, &SpotLightSystem::OnSpotLightComponentUpdated, SpotLightSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<SpotLightComponent, &SpotLightSystem::OnSpotLightComponentDestroy, SpotLightSystem>>(this);
	}

	void SpotLightSystem::OnSpotLightComponentAdded(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& billboardComp = entity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/SpotLight.png.zasset");
		entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
		{
			boundsComp.BoundsCalculationFuncs[entt::type_hash<SpotLightComponent>::value()] = [](Entity entity)
			{
				const Mat4 worldTransform = entity.GetWorldTransform();
			   const auto& lightComp = entity.GetComponent<SpotLightComponent>();
			   const Sphere sphere{ Math::GetTranslationFromTransform(worldTransform), lightComp.Range * 0.5f };
			   return sphere;
			};
		});
	}

	void SpotLightSystem::OnSpotLightComponentUpdated(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		auto& lightComp = entity.GetComponent<SpotLightComponent>();
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ShadowType"_hs)
		{
			// TODO:
		}
		else if (changeComp.FieldID == "Range"_hs)
		{
			entity.PatchComponent<BoundsComponent>();
		}
	}

	void SpotLightSystem::OnSpotLightComponentDestroy(Scene& scene, entt::entity e) const
	{
		Entity entity{ e, scene.shared_from_this() };
		entity.RemoveComponentIfExist<BillboardComponent>();
		if (entity.HasComponent<BoundsComponent>())
		{
			entity.PatchComponent<BoundsComponent>([](BoundsComponent& boundsComp)
			{
				boundsComp.BoundsCalculationFuncs.erase(entt::type_hash<SpotLightComponent>::value());
			});
		}
	}

	static void OnBoundsObserverUpdate(Scene& scene, Entity entity)
	{
		entity.UpdateBounds();
	}

	void BoundsSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentObserver(&OnBoundsObserverUpdate, entt::collector.update<BoundsComponent>().update<TransformComponent>());
	}

	void ParticleUpdateSystem::OnUpdateEditor(DeltaTime dt)
	{
		SystemBase::OnUpdateEditor(dt);

		auto particleView = GetScene()->GetComponentView<ParticleSystemComponent>();
		for (const auto e : particleView)
		{
			auto [particleComp] = particleView.get(e);
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		}
	}

	void ParticleUpdateSystem::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateEditor(dt);
	}

	void ParticlePreviewUpdateSystem::OnUpdateEditor(DeltaTime dt)
	{
		ParticleUpdateSystem::OnUpdateEditor(dt);

		auto particleView = GetScene()->GetComponentView<ParticleSystemDetailComponent>();
		for (const auto e : particleView)
		{
			auto [particleComp] = particleView.get(e);
			if (particleComp.Instance)
			{
				particleComp.Instance->OnUpdate(dt);
			}
		}
	}

	void ParticlePreviewUpdateSystem::OnUpdateRuntime(DeltaTime dt)
	{
		OnUpdateEditor(dt);
	}

	void ScriptSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentEventBinder<ComponentUpdatedBinder<ScriptComponent, &ScriptSystem::OnScriptComponentUpdated, ScriptSystem>>(this);
		RegisterComponentEventBinder<ComponentDestroyBinder<ScriptComponent, &ScriptSystem::OnScriptComponentDestroy, ScriptSystem>>(this);
	}

	void ScriptSystem::OnUpdateRuntime(DeltaTime dt)
	{
		SystemBase::OnUpdateRuntime(dt);

		if (GetWorld()->IsSimulation()) return;

		auto scriptView = GetScene()->GetComponentView<ScriptComponent>();
		for (const auto e : scriptView)
		{
			const Entity entity{ e, GetScene() };
			ScriptEngine::OnUpdateEntity(entity, dt);
		}
	}

	void ScriptSystem::OnPlayStart()
	{
		ScriptEngine::SetSceneContext(GetScene());
		auto scriptView = GetScene()->GetComponentView<ScriptComponent>();
		for (const auto e : scriptView)
		{
			const Entity entity{ e, GetScene() };
			ScriptEngine::InstantiateEntityClass(entity);
			ScriptEngine::OnCreateEntity(entity);
		}
	}

	void ScriptSystem::OnPlayStop()
	{
		ScriptEngine::SetSceneContext(GetScene());
	}

	void ScriptSystem::OnScriptComponentUpdated(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		const auto& changeComp = entity.GetComponent<FieldChangeComponent>();
		if (changeComp.FieldID == "ClassName"_hs)
		{
			ScriptEngine::OnScriptClassChanged(entity);
		}
	}

	void ScriptSystem::OnScriptComponentDestroy(Scene& scene, entt::entity e) const
	{
		const Entity entity{ e, scene.shared_from_this() };
		ScriptEngine::OnDestroyEntity(entity);
	}

	static void UpdatePhysicsActorsTransform(const Scene& scene, const PhysXScene* physicsScene, const Entity& entity)
	{
		if (const auto* actor = physicsScene->GetActor(entity))
		{
			Vec3 translation, rotation, scale;
			entity.GetWorldTransform(translation, rotation, scale);
			actor->SetTransform(translation, rotation);

			for (const UUID childID : entity.GetChildren())
			{
				if (const Entity child = scene.GetEntityByUUID(childID))
				{
					UpdatePhysicsActorsTransform(scene, physicsScene, child);
				}
			}
		}
	}

	static void OnPhysicsActorsObserverUpdate(Scene& scene, Entity entity)
	{
		if (const auto* physicsScene = scene.GetPhysicsScene())
		{
			UpdatePhysicsActorsTransform(scene, physicsScene, entity);
		}
	}

	static void OnPhysicsCollidersObserverUpdate(Scene& scene, Entity entity)
	{
		if (const auto* physicsScene = scene.GetPhysicsScene())
		{
			Entity current = entity;
			Entity parent;
			Mat4 transform = entity.GetTransform();
			while ((parent = current.GetParentEntity()))
			{
				if (parent.HasComponent<RigidBodyComponent>())
				{
					break;
				}

				current = parent;
				transform = parent.GetTransform() * transform;
			}

			if (const auto* actor = physicsScene->GetActor(parent))
			{
				const auto* colliders = actor->GetCollidersByEntity(entity);
				if (colliders)
				{
					for (const auto& collider : *colliders)
					{
						collider->SetTransform(transform);
					}
				}
			}
		}
	}

	static void OnCharacterControllerObserverUpdate(Scene& scene, Entity entity)
	{
		if (const auto* physicsScene = scene.GetPhysicsScene())
		{
			if (const auto* controller = physicsScene->GetCharacterController(entity))
			{
				const auto& controllerComp = entity.GetComponent<CharacterControllerComponent>();
				// TODO: Change to GetWorldTranslation
				controller->SetTranslation(entity.GetTranslation() + controllerComp.Offset);
			}
		}
	}

	void PhysicsSystem::OnCreate()
	{
		SystemBase::OnCreate();

		RegisterComponentObserver(&OnPhysicsActorsObserverUpdate, entt::collector.update<TransformComponent>().where<RigidBodyComponent>(ExcludeComponents<IgnoreSyncTransformComponent>));
		RegisterComponentObserver(&OnPhysicsCollidersObserverUpdate, entt::collector.update<TransformComponent>().where<ChildColliderComponent>());
		RegisterComponentObserver(&OnCharacterControllerObserverUpdate, entt::collector.update<TransformComponent>().where<CharacterControllerComponent>());
	}

	void PhysicsSystem::OnUpdateRuntime(DeltaTime dt)
	{
		SystemBase::OnUpdateRuntime(dt);

		const auto scene = GetScene();

		// Clear tag component added in PhysXActor::SynchronizeTransform after observer update
		scene->ClearTagComponents<IgnoreSyncTransformComponent>();
		scene->GetPhysicsScene()->Simulate(dt);
	}

	void PhysicsSystem::OnPlayStart()
	{
		const auto scene = GetScene();
		auto* physicsScene = scene->CreatePhysicsScene();

		auto rigidBodyView = scene->GetComponentView<RigidBodyComponent>();
		for (const auto e : rigidBodyView)
		{
			const Entity entity{ e, scene };
			physicsScene->CreateActor(entity);
		}

		ValidateColliders();

		auto characterControllerView = scene->GetComponentView<CharacterControllerComponent>();
		for (const auto e : characterControllerView)
		{
			const Entity entity{ e, scene };
			physicsScene->CreateCharacterController(entity);
		}

#ifdef ZE_DEBUG
		const auto& settings = PhysicsEngine::GetSettings();
		if (settings.bDebugOnPlay)
		{
			if (settings.DebugType == PhysXDebugType::LiveDebug)
			{
				PhysXDebugger::StartDebugging();
			}
			else
			{
				// TODO: PhysX debugger output path
				PhysXDebugger::StartDebugging(AssetRegistry::GetProjectDirectory());
			}
		}
#endif
	}

	void PhysicsSystem::ValidateColliders()
	{
		const auto scene = GetScene();
		auto boxView = scene->GetComponentView<BoxColliderComponent>(ExcludeComponents<RigidBodyComponent>);
		for (const auto e : boxView)
		{
			const Entity entity{ e, scene };
			if (!entity.HasComponent<ChildColliderComponent>())
			{
				ZE_CORE_WARN("Entity '{0}' has BoxColliderComponent but no RigidBodyComponent, so it will not participate in any physics interaction", entity.GetName());
			}
		}

		auto sphereView = scene->GetComponentView<SphereColliderComponent>(ExcludeComponents<RigidBodyComponent>);
		for (const auto e : sphereView)
		{
			const Entity entity{ e, scene };

			if (!entity.HasComponent<ChildColliderComponent>())
			{
				ZE_CORE_WARN("Entity '{0}' has SphereColliderComponent but no RigidBodyComponent, so it will not participate in any physics interaction", entity.GetName());
			}
		}

		auto capsuleView = scene->GetComponentView<CapsuleColliderComponent>(ExcludeComponents<RigidBodyComponent>);
		for (const auto e : capsuleView)
		{
			const Entity entity{ e, scene };

			if (!entity.HasComponent<ChildColliderComponent>())
			{
				ZE_CORE_WARN("Entity '{0}' has CapsuleColliderComponent but no RigidBodyComponent, so it will not participate in any physics interaction", entity.GetName());
			}
		}
	}

	void PhysicsSystem::OnPlayStop()
	{
#ifdef ZE_DEBUG
		if (PhysicsEngine::GetSettings().bDebugOnPlay)
		{
			PhysXDebugger::StopDebugging();
		}
		GetScene()->DestroyPhysicsScene();
#endif
	}

	void PhysicsSystem::OnSimulationStart()
	{
		OnPlayStart();
	}

	void PhysicsSystem::OnSimulationStop()
	{
		OnPlayStop();
	}

	void PhysicsSystem2D::OnUpdateRuntime(DeltaTime dt)
	{
		SystemBase::OnUpdateRuntime(dt);

		const I32 velocityIterations = 6;
		const I32 positionIterations = 2;
		m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

		auto rb2dGroup = GetScene()->GetComponentGroup<RigidBody2DComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : rb2dGroup)
		{
			auto [rb2dComp, transformComp] = rb2dGroup.get(e);
			// Retrieve transform from Box2D
			const b2Body* body = static_cast<b2Body*>(rb2dComp.RuntimeBody);
			const auto& position = body->GetPosition();
			transformComp.Translation.x = position.x;
			transformComp.Translation.y = position.y;
			transformComp.Rotation.z = glm::degrees(body->GetAngle());
		}
	}

	static b2BodyType RigidBody2DTypeToBox2DBody(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:	return b2_staticBody;
			case RigidBody2DComponent::BodyType::Dynamic:	return b2_dynamicBody;
			case RigidBody2DComponent::BodyType::Kinematic:	return b2_kinematicBody;
		}

		ZE_CORE_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

	void PhysicsSystem2D::OnPlayStart()
	{
		const b2Vec2 gravity = { 0.0f, -9.8f };
		m_PhysicsWorld = new b2World(gravity);
		auto rb2dGroup = GetScene()->GetComponentGroup<RigidBody2DComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : rb2dGroup)
		{
			auto [rb2dComp, transformComp] = rb2dGroup.get(e);
			b2BodyDef bodyDef;
			bodyDef.type = RigidBody2DTypeToBox2DBody(rb2dComp.Type);
			bodyDef.position.Set(transformComp.Translation.x, transformComp.Translation.y);
			bodyDef.angle = glm::radians(transformComp.Rotation.z);

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2dComp.bFixedRotation);
			rb2dComp.RuntimeBody = body;

			const Entity entity{ e, GetScene() };
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				const auto& bc2dComp = entity.GetComponent<BoxCollider2DComponent>();

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
				const auto& cc2dComp = entity.GetComponent<CircleCollider2DComponent>();

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
		}
	}

	void PhysicsSystem2D::OnPlayStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void PhysicsSystem2D::OnSimulationStart()
	{
		OnPlayStart();
	}

	void PhysicsSystem2D::OnSimulationStop()
	{
		OnPlayStop();
	}

	void CommandSystem::OnPlayStart()
	{
		Console::Get().ResetAllVariableValues();
	}

	void CommandSystem::OnPlayStop()
	{
		Console::Get().ResetAllVariableValues();
	}

	void CommandSystem::OnSimulationStart()
	{
		OnPlayStart();
	}

	void CommandSystem::OnSimulationStop()
	{
		OnPlayStop();
	}

}
