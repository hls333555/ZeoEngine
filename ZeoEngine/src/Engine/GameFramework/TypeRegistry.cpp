#include "ZEpch.h"
#include "Engine/GameFramework/TypeRegistry.h"

#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	using namespace Reflection;

	void TypeRegistry::Init()
	{
		RegisterBasicTypes();
		RegisterComponents();
	}

	void TypeRegistry::RegisterBasicTypes()
	{
		entt::meta<I32>()
			.type()
			.prop(Inherent)
			.conv<U8>()
			.conv<U32>()
			.conv<U64>()
			.conv<I8>()
			.conv<I32>()
			.conv<I64>()
			.conv<float>()
			.conv<double>();

		entt::meta<float>()
			.type()
			.prop(Inherent)
			.conv<double>();

		entt::meta<double>()
			.type()
			.prop(Inherent)
			.conv<float>();

		entt::meta<typename std::vector<bool>::reference>().conv<bool>();
	}

	void TypeRegistry::RegisterComponents()
	{
#pragma region Core
		RegisterComponent<CoreComponent>("Core", Inherent, HideComponentHeader)
			.Data<&CoreComponent::Name>("Name");

		RegisterComponent<IDComponent>("ID", Inherent, Transient);

		RegisterComponent<TransformComponent>("Transform", Inherent)
			.Data<&TransformComponent::Translation>("Translation")
			.Data<&TransformComponent::SetRotationToRadians, &TransformComponent::GetRotationAsDegrees>("Rotation", std::make_pair(ClampMin, -360.0f), std::make_pair(ClampMax, 360.0f), ClampOnlyDuringDragging)
			.Data<&TransformComponent::Scale>("Scale", std::make_pair(DragSensitivity, 0.1f));
#pragma endregion

#pragma region Rendering
		RegisterEnum<SceneCamera::ProjectionType>()
			.Data<SceneCamera::ProjectionType::Perspective>("Perspective")
			.Data<SceneCamera::ProjectionType::Orthographic>("Orthographic");

		RegisterComponent<CameraComponent>("Camera", std::make_pair(Category, "Rendering"))
			.Data<&CameraComponent::bIsPrimary>("IsPrimary")
			.Data<&CameraComponent::bFixedAspectRatio>("FixedAspectRatio")
			.Data<&CameraComponent::SetProjectionType, &CameraComponent::GetProjectionType>("ProjectionType")
			.Data<&CameraComponent::SetPerspectiveVerticalFOV, &CameraComponent::GetPerspectiveVerticalFOV>("VerticalFOV", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, "ProjectionType != Perspective"))
			.Data<&CameraComponent::SetPerspectiveNearClip, &CameraComponent::GetPerspectiveNearClip>("PerspectiveNear", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, "ProjectionType != Perspective"))
			.Data<&CameraComponent::SetPerspectiveFarClip, &CameraComponent::GetPerspectiveFarClip>("PerspectiveFar", std::make_pair(Category, "Perspective"), std::make_pair(HideCondition, "ProjectionType != Perspective"))
			.Data<&CameraComponent::SetOrthographicSize, &CameraComponent::GetOrthographicSize>("Size", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, "ProjectionType != Orthographic"))
			.Data<&CameraComponent::SetOrthographicNearClip, &CameraComponent::GetOrthographicNearClip>("OrthographicNear", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, "ProjectionType != Orthographic"))
			.Data<&CameraComponent::SetOrthographicFarClip, &CameraComponent::GetOrthographicFarClip>("OrthographicFar", std::make_pair(Category, "Orthographic"), std::make_pair(HideCondition, "ProjectionType != Orthographic"));

		RegisterComponent<SpriteRendererComponent>("Sprite Renderer", std::make_pair(Category, "Rendering"))
			.Data<&SpriteRendererComponent::TintColor>("TintColor")
			.Data<&SpriteRendererComponent::TextureAsset>("TextureAsset")
			.Data<&SpriteRendererComponent::TextureTiling>("TextureTiling")
			.Data<&SpriteRendererComponent::SortingOrder>("SortingOrder");

		RegisterComponent<CircleRendererComponent>("Circle Renderer", std::make_pair(Category, "Rendering"))
			.Data<&CircleRendererComponent::Color>("Color")
			.Data<&CircleRendererComponent::Thickness>("Thickness", std::make_pair(DragSensitivity, 0.025f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&CircleRendererComponent::Fade>("Fade", std::make_pair(DragSensitivity, 0.00025f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&CircleRendererComponent::SortingOrder>("SortingOrder");

		RegisterComponent<MeshRendererComponent>("Mesh Renderer", std::make_pair(Category, "Rendering"))
			.Data<&MeshRendererComponent::MeshAsset>("MeshAsset")
			.Data<nullptr, &MeshRendererComponent::GetMaterials>("MaterialSlots", FixedSizeContainer, CustomElementName);

		RegisterComponent<MeshPreviewComponent>("Mesh Detail", Inherent, HideComponentHeader)
			.Data<nullptr, &MeshPreviewComponent::GetMaterials>("MaterialSlots", FixedSizeContainer, CustomElementName);

		RegisterComponent<MaterialPreviewComponent>("Material Detail", Inherent, HideComponentHeader)
			.Data<&MaterialPreviewComponent::SetShaderVariant, &MaterialPreviewComponent::GetShaderVariant>("ShaderVariant", HiddenInEditor)
			.Data<&MaterialPreviewComponent::SetShader, &MaterialPreviewComponent::GetShader>("Shader");

		RegisterEnum<LightComponent::LightType>()
			.Data<LightComponent::LightType::DirectionalLight>("DirectionalLight")
			.Data<LightComponent::LightType::PointLight>("PointLight")
			.Data<LightComponent::LightType::SpotLight>("SpotLight");

		RegisterEnum<Light::ShadowType>()
			.Data<Light::ShadowType::HardShadow>("HardShadow")
			.Data<Light::ShadowType::PCF>("PCF")
			.Data<Light::ShadowType::PCSS>("PCSS");

		RegisterComponent<LightComponent>("Light", std::make_pair(Category, "Rendering"))
			.Data<&LightComponent::Type>("Type")
			.Data<&LightComponent::SetColor, &LightComponent::GetColor>("Color")
			.Data<&LightComponent::SetIntensity, &LightComponent::GetIntensity>("Intensity", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f))
			.Data<&LightComponent::SetRange, &LightComponent::GetRange>("Range", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "Type == DirectionalLight"))
			.Data<&LightComponent::SetCutoffToRadians, &LightComponent::GetCutoffAsDegrees>("CutoffAngle", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 89.0f), std::make_pair(HideCondition, "Type != SpotLight"))
			.Data<&LightComponent::SetCastShadow, &LightComponent::IsCastShadow>("CastShadow")
			.Data<&LightComponent::SetShadowType, &LightComponent::GetShadowType>("ShadowType", std::make_pair(HideCondition, "CastShadow == false"))
			.Data<&LightComponent::SetDepthBias, &LightComponent::GetDepthBias>("DepthBias", std::make_pair(DragSensitivity, 0.001f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Data<&LightComponent::SetNormalBias, &LightComponent::GetNormalBias>("NormalBias", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Data<&LightComponent::SetFilterSize, &LightComponent::GetFilterSize>("FilterSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			.Data<&LightComponent::SetLightSize, &LightComponent::GetLightSize>("LightSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			.Data<&LightComponent::SetCascadeCount, &LightComponent::GetCascadeCount>("CascadeCount", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 1), std::make_pair(ClampMax, 4), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			.Data<&LightComponent::SetCascadeBlendThreshold, &LightComponent::GetCascadeBlendThreshold>("CascadeBlendThreshold", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			.Data<&LightComponent::SetMaxShadowDistance, &LightComponent::GetMaxShadowDistance>("MaxShadowDistance", std::make_pair(DragSensitivity, 1.0f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			.Data<&LightComponent::SetCascadeSplitLambda, &LightComponent::GetCascadeSplitLambda>("CascadeSplitLambda", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, "CastShadow == false || Type != DirectionalLight"));

		RegisterComponent<BillboardComponent>("Billboard", Inherent, Transient);

		RegisterEnum<SamplerType>()
			.Data<SamplerType::BilinearRepeat>("BilinearRepeat")
			.Data<SamplerType::BilinearClamp>("BilinearClamp")
			.Data<SamplerType::PointRepeat>("PointRepeat")
			.Data<SamplerType::PointClamp>("PointClamp");

		RegisterComponent<TexturePreviewComponent>("Texture Detail", Inherent, HideComponentHeader)
			.Data<&TexturePreviewComponent::SetSRGB, &TexturePreviewComponent::IsSRGB>("SRGB")
			.Data<&TexturePreviewComponent::SetSamplerType, &TexturePreviewComponent::GetSamplerType>("SamplerType")
			.Data<&TexturePreviewComponent::SetGenerateMipmaps, &TexturePreviewComponent::ShouldGenerateMipmaps>("GenerateMipmaps");
#pragma endregion

#pragma region Scripts
		RegisterComponent<ScriptComponent>("Script", std::make_pair(Category, "Scripts"))
			.Data<&ScriptComponent::ClassName>("ClassName");

		RegisterComponent<NativeScriptComponent>("Native Script", std::make_pair(Category, "Scripts"));
#pragma endregion

#pragma region Effects
		RegisterComponent<ParticleSystemComponent>("Particle System", std::make_pair(Category, "Effects"))
			.Data<&ParticleSystemComponent::ParticleTemplateAsset>("ParticleTemplateAsset")
			.Data<&ParticleSystemComponent::PositionOffset>("PositionOffset");

		RegisterEnum<ParticleVariationType>()
			.Data<ParticleVariationType::Constant>("Constant")
			.Data<ParticleVariationType::RandomInRange>("RandomInRange")
			.Data<ParticleVariationType::UniformInRange>("UniformInRange");

#define REGISTER_PARTICLE_VARIATION_TYPE(s)				\
		RegisterStruct<s>(ZE_STRINGIFY(s))				\
			.Data<&s::VariationType>("VariationType")	\
			.Data<&s::Val1>("Value")					\
			.Data<&s::Val2>("ValueHigh", std::make_pair(HideCondition, "VariationType == Constant"))

		REGISTER_PARTICLE_VARIATION_TYPE(ParticleInt);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleFloat);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleVec2);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleVec3);
		REGISTER_PARTICLE_VARIATION_TYPE(ParticleColor);

		RegisterStruct<BurstData>("BurstData")
			.Data<&BurstData::Time>("Time", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(Tooltip, u8"标准化的时间点：[0,1]"))
			.Data<&BurstData::Amount>("Amount", std::make_pair(Tooltip, u8"在该时间点一次性生成的粒子数"));

		RegisterComponent<ParticleSystemPreviewComponent>("Particle System Detail", Inherent, HideComponentHeader)
			.Data<&ParticleSystemPreviewComponent::SetLocalSpace, &ParticleSystemPreviewComponent::IsLocalSpace>("IsLocalSpace", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"是否在局部空间模拟粒子"))
			.Data<&ParticleSystemPreviewComponent::SetLoopCount, &ParticleSystemPreviewComponent::GetLoopCount>("LoopCount", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"循环次数。若小于等于0, 则为无限循环"))
			.Data<&ParticleSystemPreviewComponent::SetLoopDuration, &ParticleSystemPreviewComponent::GetLoopDuration>("LoopDuration", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每次循环的时长"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetSpawnRate>("SpawnRate", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetBurstList>("BurstList", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"每个时间点一次性生成的粒子数列表"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetInitialPosition>("InitialPosition", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始位置"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetInitialRotation>("InitialRotation", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始旋转"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetRotationRate>("RotationRate", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的旋转速度"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetSizeBegin>("SizeBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始大小"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetSizeEnd>("SizeEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的最终大小"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetInitialVelocity>("InitialVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始速度"))
			.Data<&ParticleSystemPreviewComponent::SetInheritVelocityRatio, &ParticleSystemPreviewComponent::GetInheritVelocityRatio>("InheritVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"决定粒子所能从发射器继承的速度比率。该变量只有在非局部空间有效"), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetColorBegin>("ColorBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的初始颜色"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetColorEnd>("ColorEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的最终颜色"))
			.Data<nullptr, &ParticleSystemPreviewComponent::GetLifetime>("Lifetime", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"粒子的生命周期"))
			.Data<&ParticleSystemPreviewComponent::SetTexture, &ParticleSystemPreviewComponent::GetTexture>("Texture", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"粒子的材质贴图"))
			.Data<&ParticleSystemPreviewComponent::SetSubImageSize, &ParticleSystemPreviewComponent::GetSubImageSize>("SubImageSize", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"决定如何分割贴图来用于UV动画。x为列数，y为行数"), std::make_pair(ClampMin, 0.0f))
			.Data<&ParticleSystemPreviewComponent::SetMaxParticles, &ParticleSystemPreviewComponent::GetMaxParticles>("MaxParticles", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"最多生成的粒子数"), std::make_pair(ClampMin, 0));
#pragma endregion

#pragma region Physics
		RegisterEnum<Rigidbody2DComponent::BodyType>()
			.Data<Rigidbody2DComponent::BodyType::Static>("Static")
			.Data<Rigidbody2DComponent::BodyType::Dynamic>("Dynamic")
			.Data<Rigidbody2DComponent::BodyType::Kinematic>("Kinematic");

		RegisterComponent<Rigidbody2DComponent>("Rigidbody 2D", std::make_pair(Category, "Physics"))
			.Data<&Rigidbody2DComponent::Type>("Type")
			.Data<&Rigidbody2DComponent::bFixedRotation>("FixedRotation");

		RegisterComponent<BoxCollider2DComponent>("Box Collider 2D", std::make_pair(Category, "Physics"))
			.Data<&BoxCollider2DComponent::Offset>("Offset")
			.Data<&BoxCollider2DComponent::Size>("Size", std::make_pair(Tooltip, u8"碰撞大小，该值为缩放倍数"))
			.Data<&BoxCollider2DComponent::Density>("Density", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&BoxCollider2DComponent::Friction>("Friction", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&BoxCollider2DComponent::Restitution>("Restitution", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&BoxCollider2DComponent::RestitutionThreshold>("RestitutionThreshold", std::make_pair(Category, "Physics Material"), std::make_pair(Tooltip, u8"弹力阈值，速度高于该值时的碰撞将会反弹"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f));

		RegisterComponent<CircleCollider2DComponent>("Circle Collider 2D", std::make_pair(Category, "Physics"))
			.Data<&CircleCollider2DComponent::Offset>("Offset")
			.Data<&CircleCollider2DComponent::Radius>("Radius")
			.Data<&CircleCollider2DComponent::Density>("Density", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&CircleCollider2DComponent::Friction>("Friction", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&CircleCollider2DComponent::Restitution>("Restitution", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Data<&CircleCollider2DComponent::RestitutionThreshold>("RestitutionThreshold", std::make_pair(Category, "Physics Material"), std::make_pair(Tooltip, u8"弹力阈值，速度高于该值时的碰撞将会反弹"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f));
#pragma endregion

#pragma region Misc
		RegisterComponent<BoundsComponent>("Bounds", Inherent, Transient);
#pragma endregion

#ifndef DOCTEST_CONFIG_DISABLE
		RegisterEnum<TestComponent::TestEnum>()
			.Data<TestComponent::TestEnum::TestEnum1>("TestEnum1")
			.Data<TestComponent::TestEnum::TestEnum2>("TestEnum2")
			.Data<TestComponent::TestEnum::TestEnum3>("TestEnum3");

		RegisterEnum<TestComponent::TestEnumClass>()
			.Data<TestComponent::TestEnumClass::TestEnumClass1>("TestEnumClass1")
			.Data<TestComponent::TestEnumClass::TestEnumClass2>("TestEnumClass2")
			.Data<TestComponent::TestEnumClass::TestEnumClass3>("TestEnumClass3");

		RegisterStruct<TestComponent::TestStruct>("TestStruct")
			.Data<&TestComponent::TestStruct::EnumVar>("EnumVar")
			.Data<&TestComponent::TestStruct::I32Var>("I32Var");

		RegisterStruct<TestComponent::TestNestedStruct>("TestNestedStruct")
			.Data<&TestComponent::TestNestedStruct::TestStructVar>("TestStructVar")
			.Data<&TestComponent::TestNestedStruct::FloatVar>("FloatVar");

		RegisterComponent<TestComponent>("Test")
			.Data<&TestComponent::BoolVar>("BoolVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::Ui8Var>("Ui8Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::Ui32Var>("Ui32Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::Ui64Var>("Ui64Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::I8Var>("I8Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::I32Var>("I32Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::I64Var>("I64Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::FloatVar>("FloatVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::DoubleVar>("DoubleVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::EnumVar>("EnumVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::EnumClassVar>("EnumClassVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::StringVar>("StringVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::Vec2Var>("Vec2Var", std::make_pair(Category, "Basic"), std::make_pair(ClampMin, 5.0f))
			.Data<&TestComponent::Vec3Var>("Vec3Var", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::ColorVar>("ColorVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::Texture2DVar>("Texture2DVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::ParticleTemplateVar>("ParticleTemplateVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::TestStructVar>("TestStructVar", std::make_pair(Category, "Basic"))
			.Data<nullptr, &TestComponent::GetTestNestedStructGetterVar>("TestNestedStructGetterVar", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::SetShowSequenceContainers, &TestComponent::GetShowSequenceContainers>("ShowSequenceContainers", std::make_pair(Category, "Basic"))
			.Data<&TestComponent::BoolVecVar>("BoolVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::Ui8VecVar>("Ui8VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::Ui32VecVar>("Ui32VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::Ui64VecVar>("Ui64VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::I8VecVar>("I8VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::I32VecVar>("I32VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::I64VecVar>("I64VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::FloatVecVar>("FloatVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::DoubleVecVar>("DoubleVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::EnumVecVar>("EnumVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::EnumClassVecVar>("EnumClassVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::StringVecVar>("StringVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::Vec2VecVar>("Vec2VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::Vec3VecVar>("Vec3VecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::ColorVecVar>("ColorVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::Texture2DVecVar>("Texture2DVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::ParticleTemplateVecVar>("ParticleTemplateVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<&TestComponent::TestStructVecVar>("TestStructVecVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"))
			.Data<nullptr, &TestComponent::GetTestNestedStructVecGetterVar>("TestNestedStructVecGetterVar", std::make_pair(Category, "Sequence Container"), std::make_pair(HideCondition, "ShowSequenceContainers == false"));
#endif

	}

}
