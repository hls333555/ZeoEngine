#include "ZEpch.h"
#include "Engine/GameFramework/TypeRegistry.h"

#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	using namespace Reflection;

#ifndef DOCTEST_CONFIG_DISABLE
	bool NotShowSequenceContainers(IComponent* comp)
	{
		const auto* testComp = static_cast<TestComponent*>(comp);
		return !testComp->ShowSequenceContainers;
	}
#endif

	bool IsPerspectiveProjection(IComponent* comp)
	{
		auto* cameraComp = static_cast<CameraComponent*>(comp);
		return cameraComp->GetProjectionType() == SceneCamera::ProjectionType::Perspective;
	}

	bool IsOrthographicProjection(IComponent* comp)
	{
		auto* cameraComp = static_cast<CameraComponent*>(comp);
		return cameraComp->GetProjectionType() == SceneCamera::ProjectionType::Orthographic;
	}

	bool NotCastShadow(IComponent* comp)
	{
		const auto* lightComp = static_cast<LightComponentBase*>(comp);
		return !lightComp->bCastShadow;
	}

	bool NotCastShadowPCSS(IComponent* comp)
	{
		const auto* lightComp = static_cast<LightComponentBase*>(comp);
		return !lightComp->bCastShadow || lightComp->ShadowType != ShadowType::PCSS;
	}

	bool IsStaticRigidBody(IComponent* comp)
	{
		const auto* rigidBodyComp = static_cast<RigidBodyComponent*>(comp);
		return rigidBodyComp->Type == RigidBodyComponent::BodyType::Static;
	}

	std::string GetMeshRendererComponentSequenceContainerElementName(IComponent* comp, U32 fieldID, U32 index)
	{
		if (fieldID == "MaterialSlots"_hs)
		{
			const auto* meshComp = static_cast<MeshRendererComponent*>(comp);
			const auto mesh = AssetLibrary::LoadAsset<Mesh>(meshComp->MeshAsset);
			return mesh->GetMaterialNames()[index];
		}
		return {};
	}

	std::string GetMeshDetailComponentSequenceContainerElementName(IComponent* comp, U32 fieldID, U32 index)
	{
		if (fieldID == "MaterialSlots"_hs)
		{
			const auto* meshComp = static_cast<MeshDetailComponent*>(comp);
			return meshComp->LoadedMesh->GetMaterialNames()[index];
		}
		return {};
	}

	void TypeRegistry::Init()
	{
		RegisterBasicTypes();
		RegisterComponents();
		RegisterComponentSerializerExtenders();
	}

	void TypeRegistry::RegisterBasicTypes()
	{
		entt::meta<typename std::vector<bool>::reference>().conv<bool>();
	}

	void TypeRegistry::RegisterComponents()
	{
#pragma region Core
		RegisterComponent<CoreComponent>("Core", Inherent, HideComponentHeader)
			.Field<&CoreComponent::Name>("Name");

		RegisterComponent<RelationshipComponent>("Relationship", Inherent, HideComponentHeader)
			.Field<&RelationshipComponent::ParentEntity>("ParentEntity", HiddenInEditor)
			.Field<&RelationshipComponent::ChildEntities>("ChildEntities", HiddenInEditor);

		RegisterComponent<TransformComponent>("Transform", Inherent)
			.Field<&TransformComponent::Translation>("Translation", std::make_pair(DragSensitivity, 0.5f))
			.Field<&TransformComponent::Rotation>("Rotation", std::make_pair(DragSensitivity, 0.5f), std::make_pair(ClampMin, -360.0f), std::make_pair(ClampMax, 360.0f), ClampOnlyDuringDragging)
			.Field<&TransformComponent::Scale>("Scale", std::make_pair(DragSensitivity, 0.1f));

		RegisterComponent<WorldTransformComponent>("WorldTransform", Inherent, HideComponentHeader)
			.Field<&WorldTransformComponent::Translation>("Translation", HiddenInEditor)
			.Field<&WorldTransformComponent::Rotation>("Rotation", HiddenInEditor)
			.Field<&WorldTransformComponent::Scale>("Scale", HiddenInEditor);
#pragma endregion

#pragma region Rendering
		RegisterEnum<SceneCamera::ProjectionType>()
			.Field<SceneCamera::ProjectionType::Perspective>("Perspective")
			.Field<SceneCamera::ProjectionType::Orthographic>("Orthographic");

		RegisterComponent<CameraComponent>("Camera", std::make_pair(Category, "Rendering"))
			.Field<&CameraComponent::bIsPrimary>("IsPrimary")
			.Field<&CameraComponent::bFixedAspectRatio>("FixedAspectRatio")
			.Field<true, &CameraComponent::GetProjectionType>("ProjectionType")
			.Field<true, &CameraComponent::GetPerspectiveVerticalFOV>("VerticalFOV", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, &IsOrthographicProjection))
			.Field<true, &CameraComponent::GetPerspectiveNearClip>("PerspectiveNear", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, &IsOrthographicProjection))
			.Field<true, &CameraComponent::GetPerspectiveFarClip>("PerspectiveFar", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, &IsOrthographicProjection))
			.Field<true, &CameraComponent::GetOrthographicSize>("Size", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, &IsPerspectiveProjection))
			.Field<true, &CameraComponent::GetOrthographicNearClip>("OrthographicNear", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, &IsPerspectiveProjection))
			.Field<true, &CameraComponent::GetOrthographicFarClip>("OrthographicFar", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, &IsPerspectiveProjection));

		RegisterComponent<SpriteRendererComponent>("Sprite Renderer", std::make_pair(Category, "Rendering"))
			.Field<&SpriteRendererComponent::TintColor>("TintColor")
			.Field<&SpriteRendererComponent::TextureAsset>("TextureAsset")
			.Field<&SpriteRendererComponent::TextureTiling>("TextureTiling")
			.Field<&SpriteRendererComponent::SortingOrder>("SortingOrder");

		RegisterComponent<CircleRendererComponent>("Circle Renderer", std::make_pair(Category, "Rendering"))
			.Field<&CircleRendererComponent::Color>("Color")
			.Field<&CircleRendererComponent::Thickness>("Thickness", std::make_pair(DragSensitivity, 0.025f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleRendererComponent::Fade>("Fade", std::make_pair(DragSensitivity, 0.00025f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleRendererComponent::SortingOrder>("SortingOrder");

		RegisterComponent<MeshRendererComponent>("Mesh Renderer", std::make_pair(Category, "Rendering"))
			.Field<&MeshRendererComponent::MeshAsset>("MeshAsset", std::make_pair(AssetType, Mesh::TypeID()))
			.Field<&MeshRendererComponent::MaterialAssets>("MaterialSlots", FixedSizeContainer, std::make_pair(CustomElementName, &GetMeshRendererComponentSequenceContainerElementName), std::make_pair(AssetType, Material::TypeID()));

		RegisterComponent<MeshDetailComponent>("Mesh Detail", Inherent, HideComponentHeader)
			.Field<true, &MeshDetailComponent::GetMaterialAssets>("MaterialSlots", FixedSizeContainer, std::make_pair(CustomElementName, &GetMeshDetailComponentSequenceContainerElementName), std::make_pair(AssetType, Material::TypeID()));

		RegisterComponent<MaterialDetailComponent>("Material Detail", Inherent, HideComponentHeader)
			.Field<true, &MaterialDetailComponent::GetShaderVariant>("ShaderVariant", HiddenInEditor)
			.Field<true, &MaterialDetailComponent::GetShaderAsset>("ShaderAsset", std::make_pair(AssetType, Shader::TypeID()));

		RegisterEnum<SamplerType>()
			.Field<SamplerType::BilinearRepeat>("BilinearRepeat")
			.Field<SamplerType::BilinearClamp>("BilinearClamp")
			.Field<SamplerType::PointRepeat>("PointRepeat")
			.Field<SamplerType::PointClamp>("PointClamp");

		RegisterComponent<TextureDetailComponent>("Texture Detail", Inherent, HideComponentHeader)
			.Field<true, &TextureDetailComponent::IsSRGB>("SRGB")
			.Field<&TextureDetailComponent::SamplerType>("SamplerType")
			.Field<true, &TextureDetailComponent::ShouldGenerateMipmaps>("GenerateMipmaps");

		RegisterEnum<ShadowType>()
			.Field<ShadowType::HardShadow>("HardShadow")
			.Field<ShadowType::PCF>("PCF")
			.Field<ShadowType::PCSS>("PCSS");

		RegisterComponent<LightComponentBase>("Light Base", Inherent)
			.Field<&LightComponentBase::Color>("Color")
			.Field<&LightComponentBase::Intensity>("Intensity", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f))
			.Field<&LightComponentBase::bCastShadow>("CastShadow")
			.Field<&LightComponentBase::ShadowType>("ShadowType", std::make_pair(HideCondition, &NotCastShadow))
			.Field<&LightComponentBase::DepthBias>("DepthBias", std::make_pair(DragSensitivity, 0.001f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, &NotCastShadow))
			.Field<&LightComponentBase::NormalBias>("NormalBias", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, &NotCastShadow))
			.Field<&LightComponentBase::FilterSize>("FilterSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, &NotCastShadowPCSS))
			.Field<&LightComponentBase::LightSize>("LightSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, &NotCastShadowPCSS));

		RegisterComponent<DirectionalLightComponent, LightComponentBase>("Directional Light", std::make_pair(Category, "Rendering"))
			.Field<&DirectionalLightComponent::CascadeCount>("CascadeCount", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 1), std::make_pair(ClampMax, 4), std::make_pair(HideCondition, &NotCastShadow))
			.Field<&DirectionalLightComponent::CascadeBlendThreshold>("CascadeBlendThreshold", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, &NotCastShadow))
			.Field<&DirectionalLightComponent::MaxShadowDistance>("MaxShadowDistance", std::make_pair(DragSensitivity, 1.0f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, &NotCastShadow))
			.Field<&DirectionalLightComponent::CascadeSplitLambda>("CascadeSplitLambda", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, &NotCastShadow));

		RegisterComponent<PointLightComponent, LightComponentBase>("Point Light", std::make_pair(Category, "Rendering"))
			.Field<&PointLightComponent::Range>("Range", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f));

		RegisterComponent<SpotLightComponent, PointLightComponent>("Spot Light", std::make_pair(Category, "Rendering"))
			.Field<&SpotLightComponent::CutoffAngle>("CutoffAngle", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 89.0f));

#pragma endregion

#pragma region Scripts
		RegisterEnum<ScriptUpdateStage>()
			.Field<ScriptUpdateStage::Default>("Default")
			.Field<ScriptUpdateStage::PrePhysics>("PrePhysics")
			.Field<ScriptUpdateStage::PostPhysics>("PostPhysics");

		RegisterComponent<ScriptComponent>("Script", std::make_pair(Category, "Scripts"))
			.Field<&ScriptComponent::ClassName>("ClassName")
			.Field<&ScriptComponent::UpdateStage>("UpdateStage");

		RegisterComponent<NativeScriptComponent>("Native Script", std::make_pair(Category, "Scripts"));
#pragma endregion

#pragma region Effects
		RegisterComponent<ParticleSystemComponent>("Particle System", std::make_pair(Category, "Effects"))
			.Field<&ParticleSystemComponent::ParticleTemplateAsset>("ParticleTemplateAsset")
			.Field<&ParticleSystemComponent::PositionOffset>("PositionOffset");

		RegisterEnum<ParticleVariationType>()
			.Field<ParticleVariationType::Constant>("Constant")
			.Field<ParticleVariationType::RandomInRange>("RandomInRange")
			.Field<ParticleVariationType::UniformInRange>("UniformInRange");

#define REGISTER_PARTICLE_VARIATION_TYPE(s)				\
		RegisterStruct<s>(ZE_STRINGIFY(s))				\
			.Field<&s::VariationType>("VariationType")	\
			.Field<&s::Val1>("Value")					\
			.Field<&s::Val2>("ValueHigh", std::make_pair(HideCondition, "VariationType == Constant")) // TODO: Change to free function

		REGISTER_PARTICLE_VARIATION_TYPE(ParticleInt);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleFloat);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleVec2);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleVec3);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleColor);

		RegisterStruct<BurstData>("BurstData")
			.Field<&BurstData::Time>("Time", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(Tooltip, u8"标准化的时间点：[0,1]"))
			.Field<&BurstData::Amount>("Amount", std::make_pair(Tooltip, u8"在该时间点一次性生成的粒子数"));

		RegisterComponent<ParticleSystemDetailComponent>("Particle System Detail", Inherent, HideComponentHeader)
			.Field<true, &ParticleSystemDetailComponent::IsLocalSpace>("IsLocalSpace", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"是否在局部空间模拟粒子"))
			.Field<true, &ParticleSystemDetailComponent::GetLoopCount>("LoopCount", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"循环次数。若小于等于0, 则为无限循环"))
			.Field<true, &ParticleSystemDetailComponent::GetLoopDuration>("LoopDuration", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每次循环的时长"))
			.Field<true, &ParticleSystemDetailComponent::GetSpawnRate>("SpawnRate", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成"))
			.Field<true, &ParticleSystemDetailComponent::GetBurstList>("BurstList", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每个时间点一次性生成的粒子数列表"))
			.Field<true, &ParticleSystemDetailComponent::GetInitialPosition>("InitialPosition", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始位置"))
			.Field<true, &ParticleSystemDetailComponent::GetInitialRotation>("InitialRotation", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始旋转"))
			.Field<true, &ParticleSystemDetailComponent::GetRotationRate>("RotationRate", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的旋转速度"))
			.Field<true, &ParticleSystemDetailComponent::GetSizeBegin>("SizeBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始大小"))
			.Field<true, &ParticleSystemDetailComponent::GetSizeEnd>("SizeEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的最终大小"))
			.Field<true, &ParticleSystemDetailComponent::GetInitialVelocity>("InitialVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始速度"))
			.Field<true, &ParticleSystemDetailComponent::GetInheritVelocityRatio>("InheritVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"决定粒子所能从发射器继承的速度比率。该变量只有在非局部空间有效"), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<true, &ParticleSystemDetailComponent::GetColorBegin>("ColorBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始颜色"))
			.Field<true, &ParticleSystemDetailComponent::GetColorEnd>("ColorEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的最终颜色"))
			.Field<true, &ParticleSystemDetailComponent::GetLifetime>("Lifetime", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的生命周期"))
			.Field<true, &ParticleSystemDetailComponent::GetTexture>("Texture", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"粒子的材质贴图"))
			.Field<true, &ParticleSystemDetailComponent::GetSubImageSize>("SubImageSize", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"决定如何分割贴图来用于UV动画。x为列数，y为行数"), std::make_pair(ClampMin, 0.0f))
			.Field<true, &ParticleSystemDetailComponent::GetMaxParticles>("MaxParticles", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"最多生成的粒子数"), std::make_pair(ClampMin, 0));
#pragma endregion

#pragma region Physics
		RegisterEnum<RigidBody2DComponent::BodyType>()
			.Field<RigidBody2DComponent::BodyType::Static>("Static")
			.Field<RigidBody2DComponent::BodyType::Dynamic>("Dynamic")
			.Field<RigidBody2DComponent::BodyType::Kinematic>("Kinematic");

		RegisterComponent<RigidBody2DComponent>("RigidBody 2D", std::make_pair(Category, "Physics"))
			.Field<&RigidBody2DComponent::Type>("Type")
			.Field<&RigidBody2DComponent::bFixedRotation>("FixedRotation");

		RegisterComponent<BoxCollider2DComponent>("Box Collider 2D", std::make_pair(Category, "Physics"))
			.Field<&BoxCollider2DComponent::Offset>("Offset")
			.Field<&BoxCollider2DComponent::Size>("Size", std::make_pair(Tooltip, u8"碰撞大小，该值为缩放倍数"))
			.Field<&BoxCollider2DComponent::Density>("Density", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&BoxCollider2DComponent::Friction>("Friction", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&BoxCollider2DComponent::Restitution>("Restitution", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&BoxCollider2DComponent::RestitutionThreshold>("RestitutionThreshold", std::make_pair(Category, "Physics Material"), std::make_pair(Tooltip, u8"弹力阈值，速度高于该值时的碰撞将会反弹"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f));

		RegisterComponent<CircleCollider2DComponent>("Circle Collider 2D", std::make_pair(Category, "Physics"))
			.Field<&CircleCollider2DComponent::Offset>("Offset")
			.Field<&CircleCollider2DComponent::Radius>("Radius")
			.Field<&CircleCollider2DComponent::Density>("Density", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleCollider2DComponent::Friction>("Friction", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleCollider2DComponent::Restitution>("Restitution", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleCollider2DComponent::RestitutionThreshold>("RestitutionThreshold", std::make_pair(Category, "Physics Material"), std::make_pair(Tooltip, u8"弹力阈值，速度高于该值时的碰撞将会反弹"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f));

		RegisterEnum<RigidBodyComponent::BodyType>()
			.Field<RigidBodyComponent::BodyType::Static>("Static")
			.Field<RigidBodyComponent::BodyType::Dynamic>("Dynamic");

		RegisterEnum<RigidBodyComponent::CollisionDetectionType>()
			.Field<RigidBodyComponent::CollisionDetectionType::Discrete>("Discrete")
			.Field<RigidBodyComponent::CollisionDetectionType::Continuous>("Continuous")
			.Field<RigidBodyComponent::CollisionDetectionType::ContinuousSpeculative>("ContinuousSpeculative");

		RegisterComponent<RigidBodyComponent>("RigidBody", std::make_pair(Category, "Physics"))
			.Field<&RigidBodyComponent::Type>("Type")
			.Field<&RigidBodyComponent::bIsKinematic>("IsKinematic", std::make_pair(HideCondition, &IsStaticRigidBody))
			.Field<&RigidBodyComponent::bDisableSimulation>("DisableSimulation")
			.Field<&RigidBodyComponent::CollisionDetection>("CollisionDetection")
			.Field<&RigidBodyComponent::Mass>("Mass", std::make_pair(HideCondition, &IsStaticRigidBody))
			.Field<&RigidBodyComponent::LinearDamping>("LinearDamping", std::make_pair(HideCondition, &IsStaticRigidBody))
			.Field<&RigidBodyComponent::AngularDamping>("AngularDamping", std::make_pair(HideCondition, &IsStaticRigidBody))
			.Field<&RigidBodyComponent::bEnableGravity>("EnableGravity", std::make_pair(HideCondition, &IsStaticRigidBody))
			.Field<&RigidBodyComponent::bLockPositionX>("LockPositionX", std::make_pair(HideCondition, &IsStaticRigidBody), std::make_pair(Category, "Constraints"))
			.Field<&RigidBodyComponent::bLockPositionY>("LockPositionY", std::make_pair(HideCondition, &IsStaticRigidBody), std::make_pair(Category, "Constraints"))
			.Field<&RigidBodyComponent::bLockPositionZ>("LockPositionZ", std::make_pair(HideCondition, &IsStaticRigidBody), std::make_pair(Category, "Constraints"))
			.Field<&RigidBodyComponent::bLockRotationX>("LockRotationX", std::make_pair(HideCondition, &IsStaticRigidBody), std::make_pair(Category, "Constraints"))
			.Field<&RigidBodyComponent::bLockRotationY>("LockRotationY", std::make_pair(HideCondition, &IsStaticRigidBody), std::make_pair(Category, "Constraints"))
			.Field<&RigidBodyComponent::bLockRotationZ>("LockRotationZ", std::make_pair(HideCondition, &IsStaticRigidBody), std::make_pair(Category, "Constraints"))
			.Field<&RigidBodyComponent::bReceiveSleepEvents>("ReceiveSleepEvents", std::make_pair(HideCondition, &IsStaticRigidBody));

		RegisterComponent<PhysicsMaterialDetailComponent>("Physics Material Detail", Inherent, HideComponentHeader)
			.Field<true, &PhysicsMaterialDetailComponent::GetStaticFriction>("StaticFriction")
			.Field<true, &PhysicsMaterialDetailComponent::GetDynamicFriction>("DynamicFriction")
			.Field<true, &PhysicsMaterialDetailComponent::GetBounciness>("Bounciness", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f));

		RegisterComponent<ColliderComponentBase>("Collider Base", Inherent)
			.Field<&ColliderComponentBase::PhysicsMaterialAsset>("PhysicsMaterialAsset", std::make_pair(AssetType, PhysicsMaterial::TypeID()))
			.Field<&ColliderComponentBase::CollisionLayer>("CollisionLayer")
			.Field<&ColliderComponentBase::CollidesWithGroup>("CollidesWithGroup")
			.Field<&ColliderComponentBase::bEnableSimulation>("EnableSimulation")
			.Field<&ColliderComponentBase::bEnableQuery>("EnableQuery")
			.Field<&ColliderComponentBase::bIsTrigger>("IsTrigger");

		RegisterComponent<BoxColliderComponent, ColliderComponentBase>("Box Collider", std::make_pair(Category, "Physics"))
			.Field<&BoxColliderComponent::Size>("Size")
			.Field<&BoxColliderComponent::Offset>("Offset");

		RegisterComponent<SphereColliderComponent, ColliderComponentBase>("Sphere Collider", std::make_pair(Category, "Physics"))
			.Field<&SphereColliderComponent::Radius>("Radius")
			.Field<&SphereColliderComponent::Offset>("Offset");

		RegisterComponent<CapsuleColliderComponent, ColliderComponentBase>("Capsule Collider", std::make_pair(Category, "Physics"))
			.Field<&CapsuleColliderComponent::Radius>("Radius")
			.Field<&CapsuleColliderComponent::Height>("Height")
			.Field<&CapsuleColliderComponent::Offset>("Offset");

		RegisterComponent<CharacterControllerComponent>("Character Controller", std::make_pair(Category, "Physics"))
			.Field<&CharacterControllerComponent::PhysicsMaterialAsset>("PhysicsMaterialAsset", std::make_pair(AssetType, PhysicsMaterial::TypeID()))
			.Field<&CharacterControllerComponent::CollisionLayer>("CollisionLayer")
			.Field<&CharacterControllerComponent::CollidesWithGroup>("CollidesWithGroup")
			.Field<&CharacterControllerComponent::Radius>("Radius")
			.Field<&CharacterControllerComponent::Height>("Height")
			.Field<&CharacterControllerComponent::Offset>("Offset")
			.Field<&CharacterControllerComponent::SlopeLimitAngle>("SlopeLimitAngle", std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 180.0f))
			.Field<&CharacterControllerComponent::SkinThickness>("SkinThickness", std::make_pair(ClampMin, 0.0001f))
			.Field<&CharacterControllerComponent::StepOffset>("StepOffset", std::make_pair(ClampMin, 0.0f));
#pragma endregion

#ifndef DOCTEST_CONFIG_DISABLE
		RegisterEnum<TestComponent::TestEnum>()
			.Field<TestComponent::TestEnum::TestEnum1>("TestEnum1")
			.Field<TestComponent::TestEnum::TestEnum2>("TestEnum2")
			.Field<TestComponent::TestEnum::TestEnum3>("TestEnum3");

		RegisterEnum<TestComponent::TestEnumClass>()
			.Field<TestComponent::TestEnumClass::TestEnumClass1>("TestEnumClass1")
			.Field<TestComponent::TestEnumClass::TestEnumClass2>("TestEnumClass2")
			.Field<TestComponent::TestEnumClass::TestEnumClass3>("TestEnumClass3");

		RegisterComponent<TestComponent>("Test")
			.Field<&TestComponent::BoolVar>("BoolVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::U8Var>("U8Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::U16Var>("U16Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::U32Var>("U32Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::U64Var>("U64Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::I8Var>("I8Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::I16Var>("I16Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::I32Var>("I32Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::I64Var>("I64Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::FloatVar>("FloatVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::DoubleVar>("DoubleVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::EnumVar>("EnumVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::EnumClassVar>("EnumClassVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::StringVar>("StringVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::Vec2Var>("Vec2Var", std::make_pair(Category, "Basic"), std::make_pair(ClampMin, 5.0f))
			.Field<&TestComponent::Vec3Var>("Vec3Var", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::ColorVar>("ColorVar", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::TextureAssetVar>("TextureAssetVar", std::make_pair(Category, "Basic"), std::make_pair(AssetType, Texture2D::TypeID()))
			.Field<&TestComponent::MeshAssetVar>("MeshAssetVar", std::make_pair(Category, "Basic"), std::make_pair(AssetType, Mesh::TypeID()))
			.Field<true, &TestComponent::ShowSequenceContainers>("ShowSequenceContainers", std::make_pair(Category, "Basic"))
			.Field<&TestComponent::BoolVecVar>("BoolVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::U8VecVar>("U8VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::U16VecVar>("U16VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::U32VecVar>("U32VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::U64VecVar>("U64VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::I8VecVar>("I8VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::I16VecVar>("I16VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::I32VecVar>("I32VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::I64VecVar>("I64VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::FloatVecVar>("FloatVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::DoubleVecVar>("DoubleVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::EnumVecVar>("EnumVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::EnumClassVecVar>("EnumClassVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::StringVecVar>("StringVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::Vec2VecVar>("Vec2VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::Vec3VecVar>("Vec3VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::ColorVecVar>("ColorVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::TextureAssetVecVar>("TextureAssetVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(AssetType, Texture2D::TypeID()), std::make_pair(HideCondition, &NotShowSequenceContainers))
			.Field<&TestComponent::MeshAssetVecVar>("MeshAssetVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(AssetType, Mesh::TypeID()), std::make_pair(HideCondition, &NotShowSequenceContainers));
#endif

	}

	void TypeRegistry::RegisterComponentSerializerExtenders()
	{
		ComponentSerializerExtenderRegistry::RegisterComponentSerializerExtender<ScriptComponentSerializerExtender, ScriptComponent>();
	}

}
