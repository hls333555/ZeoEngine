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
		auto billboardView = GetScene()->GetComponentView<BillboardComponent>();
		for (const auto e : billboardView)
		{
			auto [billboardComp] = billboardView.get(e);
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
				const auto worldTransform = entity.GetWorldTransform();
				GetSceneRenderer()->DrawBillboard(Math::GetTranslationFromTransform(worldTransform), billboardComp.Size, billboardComp.TextureAsset, { 1.0f, 1.0f }, { 0.0f, 0.0f }, tintColor, static_cast<I32>(e));
			}
		}
	}

	void CameraVisualizerRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto cameraView = GetScene()->GetComponentView<CameraComponent>();
		for (const auto e : cameraView)
		{
			Entity entity{ e, GetScene() };
			// TODO: Replace this with FrameBuffer texture
			// Draw frustum visualizer when selected
			if (GetWorld()->GetContextEntity() == entity)
			{
				auto [cameraComp] = cameraView.get(e);
				const Mat4 invMatrix = entity.GetWorldTransform() * glm::inverse(cameraComp.Camera.GetProjection());
				DebugDrawUtils::DrawFrustum(*GetScene(), invMatrix, s_DebugDrawColor);
			}
		}
	}

	void MeshRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto meshView = GetScene()->GetComponentView<MeshRendererComponent>();
		for (const auto e : meshView)
		{
			auto [meshComp] = meshView.get(e);
			Entity entity{ e, GetScene() };
			GetSceneRenderer()->DrawMesh(entity.GetWorldTransform(), meshComp.Instance, meshComp.MaterialAssets, static_cast<I32>(e));
		}
	}

	void MeshRenderSystem::OnRenderRuntime()
	{
		OnRenderEditor(false);
	}

	void DirectionalLightRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto directionalLightView = GetScene()->GetComponentView<DirectionalLightComponent>();
		for (const auto e : directionalLightView)
		{
			auto [lightComp] = directionalLightView.get(e);
			Entity entity{ e, GetScene() };
			Vec3 translation, rotation, scale;
			entity.GetWorldTransform(translation, rotation, scale);
			GetSceneRenderer()->SetupDirectionalLight(rotation, lightComp);

			// Draw arrow visualizer when selected
			if (!bIsAssetPreview && GetWorld()->GetContextEntity() == entity)
			{
				const auto forward = Math::GetForwardVector(rotation);
				const auto endPosition = translation + glm::normalize(forward);
				DebugDrawUtils::DrawArrow(*GetScene(), translation, endPosition, s_DebugDrawColor, 0.25f);
			}
		}
	}

	void DirectionalLightRenderSystem::OnRenderRuntime()
	{
		auto directionalLightView = GetScene()->GetComponentView<DirectionalLightComponent>();
		for (const auto e : directionalLightView)
		{
			auto [lightComp] = directionalLightView.get(e);
			Entity entity{ e, GetScene() };
			Vec3 translation, rotation, scale;
			entity.GetWorldTransform(translation, rotation, scale);
			GetSceneRenderer()->SetupDirectionalLight(rotation, lightComp);
		}
	}

	void PointLightRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto pointLightView = GetScene()->GetComponentView<PointLightComponent>();
		for (const auto e : pointLightView)
		{
			auto [lightComp] = pointLightView.get(e);
			Entity entity{ e, GetScene() };
			const Mat4 worldTransform = entity.GetWorldTransform();
			const Vec3 translation = Math::GetTranslationFromTransform(worldTransform);
			GetSceneRenderer()->AddPointLight(translation, lightComp);

			// Draw sphere visualizer when selected
			if (!bIsAssetPreview && GetWorld()->GetContextEntity() == entity)
			{
				DebugDrawUtils::DrawSphereBounds(*GetScene(), translation, s_DebugDrawColor, lightComp.Range);
			}
		}
	}

	void PointLightRenderSystem::OnRenderRuntime()
	{
		auto pointLightView = GetScene()->GetComponentView<PointLightComponent>();
		for (const auto e : pointLightView)
		{
			auto [lightComp] = pointLightView.get(e);
			Entity entity{ e, GetScene() };
			const Mat4 worldTransform = entity.GetWorldTransform();
			const Vec3 translation = Math::GetTranslationFromTransform(worldTransform);
			GetSceneRenderer()->AddPointLight(translation, lightComp);
		}
	}

	void SpotLightRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto spotLightView = GetScene()->GetComponentView<SpotLightComponent>();
		for (const auto e : spotLightView)
		{
			auto [lightComp] = spotLightView.get(e);
			Entity entity{ e, GetScene() };
			Vec3 translation, rotation, scale;
			entity.GetWorldTransform(translation, rotation, scale);
			GetSceneRenderer()->AddSpotLight(translation, rotation, lightComp);

			// Draw cone visualizer when selected
			if (!bIsAssetPreview && GetWorld()->GetContextEntity() == entity)
			{
				const auto direction = Math::GetForwardVector(rotation) * lightComp.Range;
				const auto radius = tan(lightComp.GetCutoffInRadians()) * lightComp.Range;
				DebugDrawUtils::DrawCone(*GetScene(), translation, direction, s_DebugDrawColor, radius, 0.0f);
			}
		}
	}

	void SpotLightRenderSystem::OnRenderRuntime()
	{
		auto spotLightView = GetScene()->GetComponentView<SpotLightComponent>();
		for (const auto e : spotLightView)
		{
			auto [lightComp] = spotLightView.get(e);
			Entity entity{ e, GetScene() };
			Vec3 translation, rotation, scale;
			entity.GetWorldTransform(translation, rotation, scale);
			GetSceneRenderer()->AddSpotLight(translation, rotation, lightComp);
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
		if (!bDraw) return;

		// The debug drawing should be in sync with PhysXColliderShapeBase's geometry
		const auto scene = GetScene();
		auto boxView = scene->GetComponentView<BoxColliderComponent>();
		for (const auto e : boxView)
		{
			auto [boxComp] = boxView.get(e);
			const Entity entity{ e, scene };
			Mat4 transform = entity.GetWorldTransform();
			Mat4 boxTransform = glm::translate(glm::mat4(1.0), boxComp.Offset) * glm::scale(glm::mat4(1.0f), boxComp.Size);
			DebugDrawUtils::DrawBox(*scene, transform * boxTransform, s_DebugDrawColor);
		}

		auto sphereView = scene->GetComponentView<SphereColliderComponent>();
		for (const auto e : sphereView)
		{
			auto [sphereComp] = sphereView.get(e);
			const Entity entity{ e, scene };
			Mat4 transform = entity.GetWorldTransform();
			Vec3 translation, rotation, scale;
			Math::DecomposeTransform(transform, translation, rotation, scale);
			Mat4 sphereTransform = glm::translate(glm::mat4(1.0), sphereComp.Offset);
			float largestScale = glm::max(scale.x, glm::max(scale.y, scale.z));
			sphereTransform = transform * sphereTransform;
			DebugDrawUtils::DrawSphereBounds(*scene, Math::GetTranslationFromTransform(sphereTransform), s_DebugDrawColor, sphereComp.Radius * largestScale, rotation);
		}

		auto capsuleView = scene->GetComponentView<CapsuleColliderComponent>();
		for (const auto e : capsuleView)
		{
			auto [capsuleComp] = capsuleView.get(e);
			const Entity entity{ e, scene };
			Mat4 transform = entity.GetWorldTransform();
			Vec3 translation, rotation, scale;
			Math::DecomposeTransform(transform, translation, rotation, scale);
			Mat4 capsuleTransform = glm::translate(glm::mat4(1.0), capsuleComp.Offset);
			float radiusScale = glm::max(scale.x, scale.z);
			capsuleTransform = transform * capsuleTransform;
			DebugDrawUtils::DrawCapsule(*scene, Math::GetTranslationFromTransform(capsuleTransform), s_DebugDrawColor, capsuleComp.Radius * radiusScale, capsuleComp.Height * scale.y, rotation);
		}

		auto controllerView = scene->GetComponentView<CharacterControllerComponent>();
		for (const auto e : controllerView)
		{
			auto [controllerComp] = controllerView.get(e);
			const Entity entity{ e, scene };
			Mat4 transform = entity.GetWorldTransform();
			Vec3 translation, rotation, scale;
			Math::DecomposeTransform(transform, translation, rotation, scale);
			Mat4 capsuleTransform = glm::translate(glm::mat4(1.0), controllerComp.Offset);
			float radiusScale = glm::max(scale.x, scale.z);
			capsuleTransform = transform * capsuleTransform;
			DebugDrawUtils::DrawCapsule(*scene, Math::GetTranslationFromTransform(capsuleTransform), s_DebugDrawColor, controllerComp.Radius * radiusScale + controllerComp.SkinThickness, controllerComp.Height * scale.y, rotation);
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
			Entity entity{ e, GetScene() };
			Renderer2D::DrawSprite(entity.GetWorldTransform(), spriteComp, static_cast<I32>(e));
		}
	}

	void SpriteRenderSystem::OnRenderRuntime()
	{
		OnRenderEditor(false);
	}

	void CircleRenderSystem::OnRenderEditor(bool bIsAssetPreview)
	{
		auto circleView = GetScene()->GetComponentView<CircleRendererComponent>();
		for (const auto e : circleView)
		{
			auto [circleComp] = circleView.get(e);
			Entity entity{ e, GetScene() };
			Renderer2D::DrawCircle(entity.GetWorldTransform(), circleComp.Color, circleComp.Thickness, circleComp.Fade, static_cast<I32>(e));
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
		auto meshView = GetScene()->GetComponentView<MeshDetailComponent>();
		for (const auto e : meshView)
		{
			auto [meshComp] = meshView.get(e);
			Entity entity{ e, GetScene() };
			GetSceneRenderer()->DrawMesh(entity.GetWorldTransform(), meshComp.Instance, meshComp.Instance->GetMesh()->GetDefaultMaterialAssets());
		}
	}
	
}
