#include "ZEpch.h"
#include "Engine/GameFramework/TypeRegistry.h"

#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	using namespace Reflection;

	void TypeRegistry::Init()
	{
		RegisterBasicTypes();
		RegisterComponents();
		RegisterComponentHelpers();
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

		RegisterComponent<IDComponent>("ID", Inherent, Transient);

		RegisterComponent<TransformComponent>("Transform", Inherent)
			.Field<&TransformComponent::Translation>("Translation")
			.Field<&TransformComponent::Rotation>("Rotation", std::make_pair(ClampMin, -360.0f), std::make_pair(ClampMax, 360.0f), ClampOnlyDuringDragging)
			.Field<&TransformComponent::Scale>("Scale", std::make_pair(DragSensitivity, 0.1f));
#pragma endregion

#pragma region Rendering
		RegisterEnum<SceneCamera::ProjectionType>()
			.Field<SceneCamera::ProjectionType::Perspective>("Perspective")
			.Field<SceneCamera::ProjectionType::Orthographic>("Orthographic");

		RegisterComponent<CameraComponent>("Camera", std::make_pair(Category, "Rendering"))
			.Field<&CameraComponent::bIsPrimary>("IsPrimary")
			.Field<&CameraComponent::bFixedAspectRatio>("FixedAspectRatio")
			.Field<true, &CameraComponent::GetProjectionType>("ProjectionType")
			.Field<true, &CameraComponent::GetPerspectiveVerticalFOV>("VerticalFOV", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, "ProjectionType != Perspective"))
			.Field<true, &CameraComponent::GetPerspectiveNearClip>("PerspectiveNear", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, "ProjectionType != Perspective"))
			.Field<true, &CameraComponent::GetPerspectiveFarClip>("PerspectiveFar", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, "ProjectionType != Perspective"))
			.Field<true, &CameraComponent::GetOrthographicSize>("Size", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, "ProjectionType != Orthographic"))
			.Field<true, &CameraComponent::GetOrthographicNearClip>("OrthographicNear", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, "ProjectionType != Orthographic"))
			.Field<true, &CameraComponent::GetOrthographicFarClip>("OrthographicFar", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, "ProjectionType != Orthographic"));

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
			.Field<true, &MeshRendererComponent::GetMaterialAssets>("MaterialSlots", FixedSizeContainer, CustomElementName, std::make_pair(AssetType, Material::TypeID()));

		RegisterComponent<MeshPreviewComponent>("Mesh Detail", Inherent, HideComponentHeader)
			.Field<true, &MeshPreviewComponent::GetMaterialAssets>("MaterialSlots", FixedSizeContainer, CustomElementName, std::make_pair(AssetType, Material::TypeID()));

		RegisterComponent<MaterialPreviewComponent>("Material Detail", Inherent, HideComponentHeader)
			.Field<true, &MaterialPreviewComponent::GetShaderVariant>("ShaderVariant", HiddenInEditor)
			.Field<true, &MaterialPreviewComponent::GetShaderAsset>("ShaderAsset", std::make_pair(AssetType, Shader::TypeID()));

		RegisterEnum<LightComponent::LightType>()
			.Field<LightComponent::LightType::DirectionalLight>("DirectionalLight")
			.Field<LightComponent::LightType::PointLight>("PointLight")
			.Field<LightComponent::LightType::SpotLight>("SpotLight");

		RegisterEnum<Light::ShadowType>()
			.Field<Light::ShadowType::HardShadow>("HardShadow")
			.Field<Light::ShadowType::PCF>("PCF")
			.Field<Light::ShadowType::PCSS>("PCSS");

		RegisterComponent<LightComponent>("Light", std::make_pair(Category, "Rendering"))
			.Field<&LightComponent::Type>("Type")
			.Field<true, &LightComponent::GetColor>("Color")
			.Field<true, &LightComponent::GetIntensity>("Intensity", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f))
			.Field<true, &LightComponent::GetRange>("Range", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "Type == DirectionalLight"))
			.Field<true, &LightComponent::GetCutoff>("CutoffAngle", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 89.0f), std::make_pair(HideCondition, "Type != SpotLight"))
			.Field<true, &LightComponent::IsCastShadow>("CastShadow")
			.Field<true, &LightComponent::GetShadowType>("ShadowType", std::make_pair(HideCondition, "CastShadow == false"))
			.Field<true, &LightComponent::GetDepthBias>("DepthBias", std::make_pair(DragSensitivity, 0.001f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<true, &LightComponent::GetNormalBias>("NormalBias", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<true, &LightComponent::GetFilterSize>("FilterSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			.Field<true, &LightComponent::GetLightSize>("LightSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			.Field<true, &LightComponent::GetCascadeCount>("CascadeCount", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 1), std::make_pair(ClampMax, 4), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			.Field<true, &LightComponent::GetCascadeBlendThreshold>("CascadeBlendThreshold", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			.Field<true, &LightComponent::GetMaxShadowDistance>("MaxShadowDistance", std::make_pair(DragSensitivity, 1.0f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			.Field<true, &LightComponent::GetCascadeSplitLambda>("CascadeSplitLambda", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"));

		RegisterComponent<BillboardComponent>("Billboard", Inherent, Transient);

		RegisterEnum<SamplerType>()
			.Field<SamplerType::BilinearRepeat>("BilinearRepeat")
			.Field<SamplerType::BilinearClamp>("BilinearClamp")
			.Field<SamplerType::PointRepeat>("PointRepeat")
			.Field<SamplerType::PointClamp>("PointClamp");

		RegisterComponent<TexturePreviewComponent>("Texture Detail", Inherent, HideComponentHeader)
			.Field<true, &TexturePreviewComponent::IsSRGB>("SRGB")
			.Field<&TexturePreviewComponent::SamplerType>("SamplerType")
			.Field<true, &TexturePreviewComponent::ShouldGenerateMipmaps>("GenerateMipmaps");
#pragma endregion

#pragma region Scripts
		RegisterComponent<ScriptComponent>("Script", std::make_pair(Category, "Scripts"))
			.Field<&ScriptComponent::ClassName>("ClassName");

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
			.Field<&s::Val2>("ValueHigh", std::make_pair(HideCondition, "VariationType == Constant"))

		REGISTER_PARTICLE_VARIATION_TYPE(ParticleInt);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleFloat);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleVec2);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleVec3);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleColor);

		RegisterStruct<BurstData>("BurstData")
			.Field<&BurstData::Time>("Time", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(Tooltip, u8"标准化的时间点：[0,1]"))
			.Field<&BurstData::Amount>("Amount", std::make_pair(Tooltip, u8"在该时间点一次性生成的粒子数"));

		RegisterComponent<ParticleSystemPreviewComponent>("Particle System Detail", Inherent, HideComponentHeader)
			.Field<true, &ParticleSystemPreviewComponent::IsLocalSpace>("IsLocalSpace", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"是否在局部空间模拟粒子"))
			.Field<true, &ParticleSystemPreviewComponent::GetLoopCount>("LoopCount", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"循环次数。若小于等于0, 则为无限循环"))
			.Field<true, &ParticleSystemPreviewComponent::GetLoopDuration>("LoopDuration", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每次循环的时长"))
			.Field<true, &ParticleSystemPreviewComponent::GetSpawnRate>("SpawnRate", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成"))
			.Field<true, &ParticleSystemPreviewComponent::GetBurstList>("BurstList", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每个时间点一次性生成的粒子数列表"))
			.Field<true, &ParticleSystemPreviewComponent::GetInitialPosition>("InitialPosition", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始位置"))
			.Field<true, &ParticleSystemPreviewComponent::GetInitialRotation>("InitialRotation", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始旋转"))
			.Field<true, &ParticleSystemPreviewComponent::GetRotationRate>("RotationRate", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的旋转速度"))
			.Field<true, &ParticleSystemPreviewComponent::GetSizeBegin>("SizeBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始大小"))
			.Field<true, &ParticleSystemPreviewComponent::GetSizeEnd>("SizeEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的最终大小"))
			.Field<true, &ParticleSystemPreviewComponent::GetInitialVelocity>("InitialVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始速度"))
			.Field<true, &ParticleSystemPreviewComponent::GetInheritVelocityRatio>("InheritVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"决定粒子所能从发射器继承的速度比率。该变量只有在非局部空间有效"), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<true, &ParticleSystemPreviewComponent::GetColorBegin>("ColorBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始颜色"))
			.Field<true, &ParticleSystemPreviewComponent::GetColorEnd>("ColorEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的最终颜色"))
			.Field<true, &ParticleSystemPreviewComponent::GetLifetime>("Lifetime", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的生命周期"))
			.Field<true, &ParticleSystemPreviewComponent::GetTexture>("Texture", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"粒子的材质贴图"))
			.Field<true, &ParticleSystemPreviewComponent::GetSubImageSize>("SubImageSize", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"决定如何分割贴图来用于UV动画。x为列数，y为行数"), std::make_pair(ClampMin, 0.0f))
			.Field<true, &ParticleSystemPreviewComponent::GetMaxParticles>("MaxParticles", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"最多生成的粒子数"), std::make_pair(ClampMin, 0));
#pragma endregion

#pragma region Physics
		RegisterEnum<Rigidbody2DComponent::BodyType>()
			.Field<Rigidbody2DComponent::BodyType::Static>("Static")
			.Field<Rigidbody2DComponent::BodyType::Dynamic>("Dynamic")
			.Field<Rigidbody2DComponent::BodyType::Kinematic>("Kinematic");

		RegisterComponent<Rigidbody2DComponent>("Rigidbody 2D", std::make_pair(Category, "Physics"))
			.Field<&Rigidbody2DComponent::Type>("Type")
			.Field<&Rigidbody2DComponent::bFixedRotation>("FixedRotation");

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
#pragma endregion

#pragma region Misc
		RegisterComponent<BoundsComponent>("Bounds", Inherent, Transient);
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
			.Field<&TestComponent::BoolVecVar>("BoolVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::U8VecVar>("U8VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::U16VecVar>("U16VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::U32VecVar>("U32VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::U64VecVar>("U64VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::I8VecVar>("I8VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::I16VecVar>("I16VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::I32VecVar>("I32VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::I64VecVar>("I64VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::FloatVecVar>("FloatVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::DoubleVecVar>("DoubleVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::EnumVecVar>("EnumVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::EnumClassVecVar>("EnumClassVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::StringVecVar>("StringVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::Vec2VecVar>("Vec2VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::Vec3VecVar>("Vec3VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::ColorVecVar>("ColorVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::TextureAssetVecVar>("TextureAssetVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(AssetType, Texture2D::TypeID()), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Field<&TestComponent::MeshAssetVecVar>("MeshAssetVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(AssetType, Mesh::TypeID()), std::make_pair(HideCondition, "ShowSequenceContainers == false"));
#endif

	}

	void TypeRegistry::RegisterComponentHelpers()
	{
		ComponentHelperRegistry::RegisterComponentHelper<TransformComponentHelper, TransformComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<CameraComponentHelper, CameraComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<ScriptComponentHelper, ScriptComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<ParticleSystemComponentHelper, ParticleSystemComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<ParticleSystemPreviewComponentHelper, ParticleSystemPreviewComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<MeshRendererComponentHelper, MeshRendererComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<MeshPreviewComponentHelper, MeshPreviewComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<MaterialPreviewComponentHelper, MaterialPreviewComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<TexturePreviewComponentHelper, TexturePreviewComponent>();
		ComponentHelperRegistry::RegisterComponentHelper<LightComponentHelper, LightComponent>();
	}

}
