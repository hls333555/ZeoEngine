#include "ZEpch.h"
#include "Engine/GameFramework/RenderSystems.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Utils/DebugDrawUtils.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	RenderSystemBase::RenderSystemBase(WorldBase* world)
		: ISystem(world)
		, m_SceneRenderer(world->GetSceneRenderer())
	{
	}

	static Vec3 s_DebugDrawColor = { 0.7f, 0.7f, 0.7f };

	void RenderSystem::OnRenderEditor()
	{
		ZE_PROFILE_FUNC("RenderSystem::OnRenderEditor");

		RenderLights(true);
		RenderMeshes();

		// Render billboards
		GetScene()->ForEachComponentGroup<BillboardComponent>(IncludeComponents<TransformComponent>, [this](auto e, const BillboardComponent& billboardComp, const TransformComponent& transformComp)
		{
			if (billboardComp.TextureAsset)
			{
				GetSceneRenderer()->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, Vec4(1.0f), static_cast<I32>(e));
			}
		}, ExcludeComponents<DirectionalLightComponent, PointLightComponent, SpotLightComponent>);

		// Render camera frustums
		GetScene()->ForEachComponentGroup<CameraComponent>(IncludeComponents<TransformComponent>, [this](auto entity, const CameraComponent& cameraComp, const TransformComponent& transformComp)
		{
			// TODO: Replace this with FrameBuffer texture
			// Draw frustum visualizer when selected
			if (GetWorld()->GetContextEntity() == entity)
			{
				const Mat4 invMatrix = transformComp.GetTransform() * glm::inverse(cameraComp.Camera.GetProjection());
				DebugDrawUtils::DrawFrustum(*GetScene(), invMatrix, s_DebugDrawColor);
			}
		});
	}

	void RenderSystem::OnRenderRuntime()
	{
		RenderLights(false);
		RenderMeshes();
	}

	void RenderSystem::RenderLights(bool bIsEditor) const
	{
		GetScene()->ForEachComponentGroup<DirectionalLightComponent>(IncludeComponents<TransformComponent, BillboardComponent>, [this, bIsEditor](auto entity, const DirectionalLightComponent& lightComp, const TransformComponent& transformComp, const BillboardComponent& billboardComp)
		{
			const Vec3 rotationRad = transformComp.GetRotationInRadians();
			GetSceneRenderer()->SetupDirectionalLight(rotationRad, lightComp);

			if (bIsEditor)
			{
				// Draw arrow visualizer when selected
				if (GetWorld()->GetContextEntity() == entity)
				{
					const auto forward = Math::GetForwardVector(rotationRad);
					const auto endPosition = transformComp.Translation + glm::normalize(forward);
					DebugDrawUtils::DrawArrow(*GetScene(), transformComp.Translation, endPosition, s_DebugDrawColor, 0.25f);
				}
				if (billboardComp.TextureAsset)
				{
					const Vec4 tintColor = lightComp.Color;
					GetSceneRenderer()->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<I32>(entity));
				}
			}
		});

		GetScene()->ForEachComponentGroup<PointLightComponent>(IncludeComponents<TransformComponent, BillboardComponent>, [this, bIsEditor](auto entity, const PointLightComponent& lightComp, const TransformComponent& transformComp, const BillboardComponent& billboardComp)
		{
			GetSceneRenderer()->AddPointLight(transformComp.Translation, lightComp);

			if (bIsEditor)
			{
				// Draw sphere visualizer when selected
				if (GetWorld()->GetContextEntity() == entity)
				{
					DebugDrawUtils::DrawSphereBounds(*GetScene(), transformComp.Translation, s_DebugDrawColor, lightComp.Range);
				}
				if (billboardComp.TextureAsset)
				{
					const Vec4 tintColor = lightComp.Color;
					GetSceneRenderer()->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<I32>(entity));
				}
			}
		});

		GetScene()->ForEachComponentGroup<SpotLightComponent>(IncludeComponents<TransformComponent, BillboardComponent>, [this, bIsEditor](auto entity, const SpotLightComponent& lightComp, const TransformComponent& transformComp, const BillboardComponent& billboardComp)
		{
			const Vec3 rotationRad = transformComp.GetRotationInRadians();
			GetSceneRenderer()->AddSpotLight(transformComp.Translation, rotationRad, lightComp);

			if (bIsEditor)
			{
				// Draw cone visualizer when selected
				if (GetWorld()->GetContextEntity() == entity)
				{
					const auto direction = Math::GetForwardVector(rotationRad) * lightComp.Range;
					const auto radius = tan(lightComp.GetCutoffInRadians()) * lightComp.Range;
					DebugDrawUtils::DrawCone(*GetScene(), transformComp.Translation, direction, s_DebugDrawColor, radius, 0.0f);
				}
				if (billboardComp.TextureAsset)
				{
					const Vec4 tintColor = lightComp.Color;
					GetSceneRenderer()->DrawBillboard(transformComp.Translation, billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<I32>(entity));
				}
			}
		});
	}

	void RenderSystem::RenderMeshes() const
	{
		GetScene()->ForEachComponentGroup<MeshRendererComponent>(IncludeComponents<TransformComponent/*, BoundsComponent*/>, [this](auto entity, const MeshRendererComponent& meshComp, const TransformComponent& transformComp/*, const BoundsComponent& boundsComp*/)
		{
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance, meshComp.MaterialAssets, static_cast<I32>(entity));
			//DebugDrawUtils::DrawSphereBounds(GetScene(), boundsComp.Bounds.Origin, s_DebugDrawColor, boundsComp.Bounds.SphereRadius);
		});
	}

	void RenderSystem2D::OnRenderEditor()
	{
		// Render sprites
		GetScene()->ForEachComponentGroup<TransformComponent>(IncludeComponents<SpriteRendererComponent>, [](auto entity, const TransformComponent& transformComp, const SpriteRendererComponent& spriteComp)
		{
			Renderer2D::DrawSprite(transformComp.GetTransform(), spriteComp, static_cast<I32>(entity));
		}, ExcludeComponents<>,
		[](std::tuple<TransformComponent&, SpriteRendererComponent&> lhs, std::tuple<TransformComponent&, SpriteRendererComponent&> rhs)
		{
			const auto& lSpriteComp = std::get<1>(lhs);
			const auto& rSpriteComp = std::get<1>(rhs);
			return lSpriteComp.SortingOrder < rSpriteComp.SortingOrder;
		});

		// Render circles
		GetScene()->ForEachComponentGroup<CircleRendererComponent>(IncludeComponents<TransformComponent>, [](auto entity, const CircleRendererComponent& circleComp, const TransformComponent& transformComp)
		{
			Renderer2D::DrawCircle(transformComp.GetTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<I32>(entity));
		});

		// Render particle systems
		GetScene()->ForEachComponentView<ParticleSystemComponent>([](auto entity, const ParticleSystemComponent& particleComp)
		{
			if (particleComp.Instance)
			{
				particleComp.Instance->OnRender();
			}
		});
	}

	void RenderSystem2D::OnRenderRuntime()
	{
		OnRenderEditor();
	}

	void ParticlePreviewRenderSystem::OnRenderEditor()
	{
		GetScene()->ForEachComponentView<ParticleSystemDetailComponent>([](auto entity, const ParticleSystemDetailComponent& particlePreviewComp)
		{
			particlePreviewComp.Instance->OnRender();
		});
	}

	void MeshPreviewRenderSystem::OnRenderEditor()
	{
		GetScene()->ForEachComponentGroup<DirectionalLightComponent>(IncludeComponents<TransformComponent>, [this](auto entity, const DirectionalLightComponent& lightComp, const TransformComponent& transformComp)
		{
			GetSceneRenderer()->SetupDirectionalLight(transformComp.GetRotationInRadians(), lightComp);
		});
		GetScene()->ForEachComponentGroup<MeshDetailComponent>(IncludeComponents<TransformComponent/*, BoundsComponent*/>, [this](auto entity, const MeshDetailComponent& meshComp, const TransformComponent& transformComp/*, const BoundsComponent& boundsComp*/)
		{
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance, meshComp.Instance->GetMesh()->GetDefaultMaterialAssets());
			//DebugDrawUtils::DrawSphereBounds(GetScene(), boundsComp.Bounds.Origin, s_DebugDrawColor, boundsComp.Bounds.SphereRadius);
		});
	}

	void MaterialPreviewRenderSystem::OnRenderEditor()
	{
		GetScene()->ForEachComponentGroup<DirectionalLightComponent>(IncludeComponents<TransformComponent>, [this](auto entity, const DirectionalLightComponent& lightComp, const TransformComponent& transformComp)
		{
			GetSceneRenderer()->SetupDirectionalLight(transformComp.GetRotationInRadians(), lightComp);
		});
		GetScene()->ForEachComponentGroup<MeshRendererComponent>(IncludeComponents<TransformComponent>, [this](auto entity, const MeshRendererComponent& meshComp, const TransformComponent& transformComp)
		{
			GetSceneRenderer()->DrawMesh(transformComp.GetTransform(), meshComp.Instance, meshComp.MaterialAssets);
		});
	}
	
}
