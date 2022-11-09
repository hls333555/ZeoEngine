#include "ZEpch.h"
#include "Engine/GameFramework/TypeRegistry.h"

#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	using namespace Reflection;

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

		RegisterComponent<LightComponentBase>("LightComponentBase", Inherent)
			.Field<&LightComponentBase::Color>("Color")
			.Field<&LightComponentBase::Intensity>("Intensity", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f))
			.Field<&LightComponentBase::bCastShadow>("CastShadow")
			.Field<&LightComponentBase::ShadowType>("ShadowType", std::make_pair(HideCondition, "CastShadow == false"))
			.Field<&LightComponentBase::DepthBias>("DepthBias", std::make_pair(DragSensitivity, 0.001f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<&LightComponentBase::NormalBias>("NormalBias", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<&LightComponentBase::FilterSize>("FilterSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			.Field<&LightComponentBase::LightSize>("LightSize", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false || ShadowType != PCSS"));

		RegisterComponent<DirectionalLightComponent, LightComponentBase>("DirectionalLight", std::make_pair(Category, "Rendering"))
			.Field<&DirectionalLightComponent::CascadeCount>("CascadeCount", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 1), std::make_pair(ClampMax, 4), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<&DirectionalLightComponent::CascadeBlendThreshold>("CascadeBlendThreshold", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<&DirectionalLightComponent::MaxShadowDistance>("MaxShadowDistance", std::make_pair(DragSensitivity, 1.0f), std::make_pair(ClampMin, 0.0f), std::make_pair(HideCondition, "CastShadow == false"))
			.Field<&DirectionalLightComponent::CascadeSplitLambda>("CascadeSplitLambda", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(HideCondition, "CastShadow == false"));

		RegisterComponent<PointLightComponent, LightComponentBase>("PointLight", std::make_pair(Category, "Rendering"))
			.Field<&PointLightComponent::Range>("Range", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f));

		RegisterComponent<SpotLightComponent, PointLightComponent>("SpotLight", std::make_pair(Category, "Rendering"))
			.Field<&SpotLightComponent::CutoffAngle>("CutoffAngle", std::make_pair(DragSensitivity, 0.1f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 89.0f));

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
			.Field<&BurstData::Time>("Time", std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f), std::make_pair(Tooltip, u8"��׼����ʱ��㣺[0,1]"))
			.Field<&BurstData::Amount>("Amount", std::make_pair(Tooltip, u8"�ڸ�ʱ���һ�������ɵ�������"));

		RegisterComponent<ParticleSystemDetailComponent>("Particle System Detail", Inherent, HideComponentHeader)
			.Field<true, &ParticleSystemDetailComponent::IsLocalSpace>("IsLocalSpace", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"�Ƿ��ھֲ��ռ�ģ������"))
			.Field<true, &ParticleSystemDetailComponent::GetLoopCount>("LoopCount", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"ѭ����������С�ڵ���0, ��Ϊ����ѭ��"))
			.Field<true, &ParticleSystemDetailComponent::GetLoopDuration>("LoopDuration", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"ÿ��ѭ����ʱ��"))
			.Field<true, &ParticleSystemDetailComponent::GetSpawnRate>("SpawnRate", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"ÿ���ܹ����ɵ����������ñ����������ӵ������ٶȡ���С�ڵ���0��������"))
			.Field<true, &ParticleSystemDetailComponent::GetBurstList>("BurstList", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"ÿ��ʱ���һ�������ɵ��������б�"))
			.Field<true, &ParticleSystemDetailComponent::GetInitialPosition>("InitialPosition", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵĳ�ʼλ��"))
			.Field<true, &ParticleSystemDetailComponent::GetInitialRotation>("InitialRotation", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵĳ�ʼ��ת"))
			.Field<true, &ParticleSystemDetailComponent::GetRotationRate>("RotationRate", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵ���ת�ٶ�"))
			.Field<true, &ParticleSystemDetailComponent::GetSizeBegin>("SizeBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵĳ�ʼ��С"))
			.Field<true, &ParticleSystemDetailComponent::GetSizeEnd>("SizeEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵ����մ�С"))
			.Field<true, &ParticleSystemDetailComponent::GetInitialVelocity>("InitialVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵĳ�ʼ�ٶ�"))
			.Field<true, &ParticleSystemDetailComponent::GetInheritVelocityRatio>("InheritVelocity", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"�����������ܴӷ������̳е��ٶȱ��ʡ��ñ���ֻ���ڷǾֲ��ռ���Ч"), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<true, &ParticleSystemDetailComponent::GetColorBegin>("ColorBegin", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵĳ�ʼ��ɫ"))
			.Field<true, &ParticleSystemDetailComponent::GetColorEnd>("ColorEnd", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵ�������ɫ"))
			.Field<true, &ParticleSystemDetailComponent::GetLifetime>("Lifetime", std::make_pair(Category, "Particle"), std::make_pair(Tooltip, u8"���ӵ���������"))
			.Field<true, &ParticleSystemDetailComponent::GetTexture>("Texture", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"���ӵĲ�����ͼ"))
			.Field<true, &ParticleSystemDetailComponent::GetSubImageSize>("SubImageSize", std::make_pair(Category, "Renderer"), std::make_pair(Tooltip, u8"������ηָ���ͼ������UV������xΪ������yΪ����"), std::make_pair(ClampMin, 0.0f))
			.Field<true, &ParticleSystemDetailComponent::GetMaxParticles>("MaxParticles", std::make_pair(Category, "Emitter"), std::make_pair(Tooltip, u8"������ɵ�������"), std::make_pair(ClampMin, 0));
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
			.Field<&BoxCollider2DComponent::Size>("Size", std::make_pair(Tooltip, u8"��ײ��С����ֵΪ���ű���"))
			.Field<&BoxCollider2DComponent::Density>("Density", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&BoxCollider2DComponent::Friction>("Friction", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&BoxCollider2DComponent::Restitution>("Restitution", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&BoxCollider2DComponent::RestitutionThreshold>("RestitutionThreshold", std::make_pair(Category, "Physics Material"), std::make_pair(Tooltip, u8"������ֵ���ٶȸ��ڸ�ֵʱ����ײ���ᷴ��"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f));

		RegisterComponent<CircleCollider2DComponent>("Circle Collider 2D", std::make_pair(Category, "Physics"))
			.Field<&CircleCollider2DComponent::Offset>("Offset")
			.Field<&CircleCollider2DComponent::Radius>("Radius")
			.Field<&CircleCollider2DComponent::Density>("Density", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleCollider2DComponent::Friction>("Friction", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleCollider2DComponent::Restitution>("Restitution", std::make_pair(Category, "Physics Material"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f), std::make_pair(ClampMax, 1.0f))
			.Field<&CircleCollider2DComponent::RestitutionThreshold>("RestitutionThreshold", std::make_pair(Category, "Physics Material"), std::make_pair(Tooltip, u8"������ֵ���ٶȸ��ڸ�ֵʱ����ײ���ᷴ��"), std::make_pair(DragSensitivity, 0.01f), std::make_pair(ClampMin, 0.0f));
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

	void TypeRegistry::RegisterComponentSerializerExtenders()
	{
		ComponentSerializerExtenderRegistry::RegisterComponentSerializerExtender<ScriptComponentSerializerExtender, ScriptComponent>();
	}

}