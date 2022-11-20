#include "ZEpch.h"
#include "Engine/GameFramework/RenderSystems.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Utils/DebugDrawUtils.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/Console.h"
#include "Engine/Core/ConsoleVariables.h"
#include "Engine/Physics/PhysicsEngine.h"

namespace ZeoEngine {

	RenderSystemBase::RenderSystemBase(WorldBase* world)
		: ISystem(world)
		, m_SceneRenderer(world->GetSceneRenderer())
	{
	}

	static Vec3 s_DebugDrawColor = { 0.7f, 0.7f, 0.7f };

	void BillboardRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto billboardGroup = GetScene()->GetComponentGroup<BillboardComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : billboardGroup)
		{
			auto [billboardComp, transformComp] = billboardGroup.get(e);
			if (billboardComp.TextureAsset)
			{
				Entity entity{ e, GetScene() };
				Vec4 tintColor{ 1.0f };
				if (entity.HasComponent<DirectionalLightComponent>())
				{
					tintColor = entity.GetComponent<DirectionalLightComponent>().Color;
				}
				if (entity.HasComponent<PointLightComponent>())
				{
					tintColor = entity.GetComponent<PointLightComponent>().Color;
				}
				if (entity.HasComponent<SpotLightComponent>())
				{
					tintColor = entity.GetComponent<SpotLightComponent>().Color;
				}
				GetSceneRenderer()->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<I32>(e));
			}
		}
	}

	void CameraVisualizerRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto cameraGroup = GetScene()->GetComponentGroup<CameraComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : cameraGroup)
		{
			Entity entity{ e, GetScene() };
			// TODO: Replace this with FrameBuffer texture
			// Draw frustum visualizer when selected
			if (GetWorld()->GetContextEntity() == entity)
			{
				auto [cameraComp, transformComp] = cameraGroup.get(e);
				const Mat4 invMatrix = transformComp.GetTransform() * glm::inverse(cameraComp.Camera.GetProjection());
				DebugDrawUtils::DrawFrustum(*GetScene(), invMatrix, s_DebugDrawColor);
			}
		}
	}

	void MeshRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto meshGroup = GetScene()->GetComponentGroup<MeshRendererComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : meshGroup)
		{
			auto [meshComp, transformComp] = meshGroup.get(e);
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance, meshComp.MaterialAssets, static_cast<I32>(e));
		}
	}

	void MeshRenderSystem::OnRenderRuntime()
	{
		OnRenderEditor(false);
	}

	void DirectionalLightRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto directionalLightGroup = GetScene()->GetComponentGroup<DirectionalLightComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : directionalLightGroup)
		{
			auto [lightComp, transformComp] = directionalLightGroup.get(e);
			const Vec3 rotationRad = transformComp.GetRotationInRadians();
			GetSceneRenderer()->SetupDirectionalLight(rotationRad, lightComp);

			Entity entity{ e, GetScene() };
			// Draw arrow visualizer when selected
			if (!bIsAssetPreview && GetWorld()->GetContextEntity() == entity)
			{
				const auto forward = Math::GetForwardVector(rotationRad);
				const auto endPosition = transformComp.Translation + glm::normalize(forward);
				DebugDrawUtils::DrawArrow(*GetScene(), transformComp.Translation, endPosition, s_DebugDrawColor, 0.25f);
			}
		}
	}

	void DirectionalLightRenderSystem::OnRenderRuntime()
	{
		auto directionalLightGroup = GetScene()->GetComponentGroup<DirectionalLightComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : directionalLightGroup)
		{
			auto [lightComp, transformComp] = directionalLightGroup.get(e);
			const Vec3 rotationRad = transformComp.GetRotationInRadians();
			GetSceneRenderer()->SetupDirectionalLight(rotationRad, lightComp);
		}
	}

	void PointLightRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto pointLightGroup = GetScene()->GetComponentGroup<PointLightComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : pointLightGroup)
		{
			auto [lightComp, transformComp] = pointLightGroup.get(e);
			GetSceneRenderer()->AddPointLight(transformComp.Translation, lightComp);

			Entity entity{ e, GetScene() };
			// Draw sphere visualizer when selected
			if (!bIsAssetPreview && GetWorld()->GetContextEntity() == entity)
			{
				DebugDrawUtils::DrawSphereBounds(*GetScene(), transformComp.Translation, s_DebugDrawColor, lightComp.Range);
			}
		}
	}

	void PointLightRenderSystem::OnRenderRuntime()
	{
		auto pointLightGroup = GetScene()->GetComponentGroup<PointLightComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : pointLightGroup)
		{
			auto [lightComp, transformComp] = pointLightGroup.get(e);
			GetSceneRenderer()->AddPointLight(transformComp.Translation, lightComp);
		}
	}

	void SpotLightRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto spotLightGroup = GetScene()->GetComponentGroup<SpotLightComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : spotLightGroup)
		{
			auto [lightComp, transformComp] = spotLightGroup.get(e);
			const Vec3 rotationRad = transformComp.GetRotationInRadians();
			GetSceneRenderer()->AddSpotLight(transformComp.Translation, rotationRad, lightComp);

			Entity entity{ e, GetScene() };
			// Draw cone visualizer when selected
			if (!bIsAssetPreview && GetWorld()->GetContextEntity() == entity)
			{
				const auto direction = Math::GetForwardVector(rotationRad) * lightComp.Range;
				const auto radius = tan(lightComp.GetCutoffInRadians()) * lightComp.Range;
				DebugDrawUtils::DrawCone(*GetScene(), transformComp.Translation, direction, s_DebugDrawColor, radius, 0.0f);
			}
		}
	}

	void SpotLightRenderSystem::OnRenderRuntime()
	{
		auto spotLightGroup = GetScene()->GetComponentGroup<SpotLightComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : spotLightGroup)
		{
			auto [lightComp, transformComp] = spotLightGroup.get(e);
			const Vec3 rotationRad = transformComp.GetRotationInRadians();
			GetSceneRenderer()->AddSpotLight(transformComp.Translation, rotationRad, lightComp);
		}
	}

	void PhysicsDebugRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		DrawColliders(true);
		DrawWorldBounds();
	}

	void PhysicsDebugRenderSystem::OnRenderRuntime()
	{
		DrawColliders(static_cast<bool>(*Console::Get().GetVariableValue(CVAR_PHYSICS_DRAWCOLLIDERS)));
		DrawWorldBounds();
	}

	void PhysicsDebugRenderSystem::DrawColliders(bool bDraw)
	{
		const auto scene = GetScene();
		auto boxGroup = scene->GetComponentGroup<BoxColliderComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : boxGroup)
		{
			auto [boxComp, transformComp] = boxGroup.get(e);
			const Entity entity{ e, scene };
			if (bDraw)
			{
				DebugDrawUtils::DrawBox(*scene, entity.GetTranslation() + boxComp.Offset, entity.GetScale() * boxComp.Size, s_DebugDrawColor, entity.GetRotation());
			}
		}

		auto sphereGroup = scene->GetComponentGroup<SphereColliderComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : sphereGroup)
		{
			auto [sphereComp, transformComp] = sphereGroup.get(e);
			const Entity entity{ e, scene };
			if (bDraw)
			{
				DebugDrawUtils::DrawSphereBounds(*scene, entity.GetTranslation() + sphereComp.Offset, s_DebugDrawColor, sphereComp.Radius);
			}
		}

		auto capsuleGroup = scene->GetComponentGroup<CapsuleColliderComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : capsuleGroup)
		{
			auto [capsuleComp, transformComp] = capsuleGroup.get(e);
			const Entity entity{ e, scene };
			if (bDraw)
			{
				DebugDrawUtils::DrawCapsule(*scene, entity.GetTranslation() + capsuleComp.Offset, s_DebugDrawColor, capsuleComp.Radius, capsuleComp.Height, entity.GetRotation());
			}
		}
	}

	void PhysicsDebugRenderSystem::DrawWorldBounds() const
	{
		if (static_cast<bool>(*Console::Get().GetVariableValue(CVAR_PHYSICS_DRAWWORLDBOUNDS)))
		{
			const auto& settings = PhysicsEngine::GetSettings();
			DebugDrawUtils::DrawBox(*GetScene(), settings.WorldBoundsCenter, settings.WorldBoundsExtent, Vec3{ 0.0f, 0.5f, 1.0f });
		}
	}

	void ParticleSystemRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto particleView = GetScene()->GetComponentView<ParticleSystemComponent>();
		for (const auto e : particleView)
		{
			auto [particleComp] = particleView.get(e);
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
		}
	}

	void ParticleSystemRenderSystem::OnRenderRuntime()
	{
		OnRenderEditor(false);
	}

	void SpriteRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto spriteGroup = GetScene()->GetComponentGroup<SpriteRendererComponent>(IncludeComponents<TransformComponent>);
		spriteGroup.sort<SpriteRendererComponent, TransformComponent>([](std::tuple<SpriteRendererComponent&, TransformComponent&> lhs, std::tuple<SpriteRendererComponent&, TransformComponent&> rhs)
		{
			const auto& lSpriteComp = std::get<0>(lhs);
			const auto& rSpriteComp = std::get<0>(rhs);
			return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
		});
		for (const auto e : spriteGroup)
		{
			auto [spriteComp, transformComp] = spriteGroup.get(e);
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<I32>(e));
		}
	}

	void SpriteRenderSystem::OnRenderRuntime()
	{
		OnRenderEditor(false);
	}

	void CircleRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto circleGroup = GetScene()->GetComponentGroup<CircleRendererComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : circleGroup)
		{
			auto [circleComp, transformComp] = circleGroup.get(e);
			Renderer2D::DrawCircle(transformComp.GetTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<I32>(e));
		}
	}

	void CircleRenderSystem::OnRenderRuntime()
	{
		OnRenderEditor(false);
	}

	void ParticlePreviewRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto particleView = GetScene()->GetComponentView<ParticleSystemDetailComponent>();
		for (const auto e : particleView)
		{
			auto [particleComp] = particleView.get(e);
			particleComp.Instance->OnRender();
		}
	}

	void MeshPreviewRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto meshGroup = GetScene()->GetComponentGroup<MeshDetailComponent>(IncludeComponents<TransformComponent>);
		for (const auto e : meshGroup)
		{
			auto [meshComp, transformComp] = meshGroup.get(e);
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance, meshComp.Instance->GetMesh()->GetDefaultMaterialAssets());
		}
	}
	
}
