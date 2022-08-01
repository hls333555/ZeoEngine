#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionCore.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	ZE_REGISTRATION
	{
		ZCOMPONENT(CoreComponent, ZPROP(Inherent), ZPROP(HideComponentHeader))
			ZDATA(CoreComponent, Name);

		ZCOMPONENT(IDComponent, ZPROP(Inherent), ZPROP(Transient));

		ZCOMPONENT(TransformComponent, ZPROP(DisplayName, ZTEXT("Transform")), ZPROP(Tooltip, ZTEXT("变换组件")), ZPROP(Inherent))
			ZDATA(TransformComponent, Translation, ZPROP(Tooltip, ZTEXT("平移")), ZPROP(DragSensitivity, 0.1f))
			ZDATA_SETTER_GETTER(TransformComponent, Rotation, SetRotationToRadians, GetRotationAsDegrees, ZPROP(ClampMin, -360.0f), ZPROP(ClampMax, 360.0f), ZPROP(ClampOnlyDuringDragging), ZPROP(Tooltip, ZTEXT("旋转")))
			ZDATA(TransformComponent, Scale, ZPROP(Tooltip, ZTEXT("缩放")), ZPROP(DragSensitivity, 0.1f));

		ZCOMPONENT(SpriteRendererComponent, ZPROP(DisplayName, ZTEXT("Sprite Renderer")), ZPROP(Tooltip, ZTEXT("精灵渲染组件")), ZPROP(Category, ZTEXT("Rendering")))
			ZDATA(SpriteRendererComponent, TintColor)
			ZDATA(SpriteRendererComponent, TextureAsset)
			ZDATA(SpriteRendererComponent, TextureTiling)
			ZDATA(SpriteRendererComponent, SortingOrder);

		ZCOMPONENT(CircleRendererComponent, ZPROP(DisplayName, ZTEXT("Circle Renderer")), ZPROP(Tooltip, ZTEXT("圆形渲染组件")), ZPROP(Category, ZTEXT("Rendering")))
			ZDATA(CircleRendererComponent, Color)
			ZDATA(CircleRendererComponent, Thickness, ZPROP(DragSensitivity, 0.025f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(CircleRendererComponent, Fade, ZPROP(DragSensitivity, 0.00025f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(CircleRendererComponent, SortingOrder);

		ZENUM(SceneCamera::ProjectionType)
			ZENUM_DATA(SceneCamera::ProjectionType, Perspective, ZPROP(Tooltip, ZTEXT("透视相机")))
			ZENUM_DATA(SceneCamera::ProjectionType, Orthographic, ZPROP(Tooltip, ZTEXT("正交相机")));

		ZCOMPONENT(CameraComponent, ZPROP(DisplayName, ZTEXT("Camera")), ZPROP(Tooltip, ZTEXT("相机组件")), ZPROP(Category, ZTEXT("Rendering")))
			ZDATA(CameraComponent, bIsPrimary)
			ZDATA(CameraComponent, bFixedAspectRatio)
			ZDATA_SETTER_GETTER(CameraComponent, ProjectionType, SetProjectionType, GetProjectionType)
			ZDATA_SETTER_GETTER(CameraComponent, VerticalFOV, SetPerspectiveVerticalFOV, GetPerspectiveVerticalFOV, ZPROP(Category, ZTEXT("Perspective")), ZPROP(HideCondition, "ProjectionType != Perspective"))
			ZDATA_SETTER_GETTER(CameraComponent, PerspectiveNear, SetPerspectiveNearClip, GetPerspectiveNearClip, ZPROP(Category, ZTEXT("Perspective")), ZPROP(HideCondition, "ProjectionType != Perspective"))
			ZDATA_SETTER_GETTER(CameraComponent, PerspectiveFar, SetPerspectiveFarClip, GetPerspectiveFarClip, ZPROP(Category, ZTEXT("Perspective")), ZPROP(HideCondition, "ProjectionType != Perspective"))
			ZDATA_SETTER_GETTER(CameraComponent, Size, SetOrthographicSize, GetOrthographicSize, ZPROP(Category, ZTEXT("Orthographic")), ZPROP(HideCondition, "ProjectionType != Orthographic"))
			ZDATA_SETTER_GETTER(CameraComponent, OrthographicNear, SetOrthographicNearClip, GetOrthographicNearClip, ZPROP(Category, ZTEXT("Orthographic")), ZPROP(HideCondition, "ProjectionType != Orthographic"))
			ZDATA_SETTER_GETTER(CameraComponent, OrthographicFar, SetOrthographicFarClip, GetOrthographicFarClip, ZPROP(Category, ZTEXT("Orthographic")), ZPROP(HideCondition, "ProjectionType != Orthographic"));

		ZCOMPONENT(NativeScriptComponent, ZPROP(DisplayName, ZTEXT("Native Script")), ZPROP(Tooltip, ZTEXT("C++脚本组件")), ZPROP(Category, ZTEXT("Scripts")));

		ZCOMPONENT(ParticleSystemComponent, ZPROP(DisplayName, ZTEXT("Particle System")), ZPROP(Tooltip, ZTEXT("粒子系统组件")), ZPROP(Category, ZTEXT("Effects")))
			ZDATA(ParticleSystemComponent, ParticleTemplateAsset)
			ZDATA(ParticleSystemComponent, PositionOffset);

		ZENUM(ParticleVariationType)
			ZENUM_DATA(ParticleVariationType, Constant)
			ZENUM_DATA(ParticleVariationType, RandomInRange)
			ZENUM_DATA(ParticleVariationType, UniformInRange);

#define ZSTRUCT_PARTICLE_VARIATION_TYPE(_type)						\
		ZSTRUCT(_type)												\
			ZDATA(_type, VariationType)								\
			ZDATA(_type, Val1, ZPROP(DisplayName, ZTEXT("Value")))	\
			ZDATA(_type, Val2, ZPROP(DisplayName, ZTEXT("ValueHigh")), ZPROP(HideCondition, "VariationType == Constant"))

		ZSTRUCT_PARTICLE_VARIATION_TYPE(ParticleInt);
		ZSTRUCT_PARTICLE_VARIATION_TYPE(ParticleFloat);
		ZSTRUCT_PARTICLE_VARIATION_TYPE(ParticleVec2);
		ZSTRUCT_PARTICLE_VARIATION_TYPE(ParticleVec3);
		ZSTRUCT_PARTICLE_VARIATION_TYPE(ParticleColor);

		ZSTRUCT(BurstData)
			ZDATA(BurstData, Time, ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f), ZPROP(DragSensitivity, 0.01f), ZPROP(Tooltip, ZTEXT("标准化的时间点：[0,1]")))
			ZDATA(BurstData, Amount, ZPROP(Tooltip, ZTEXT("在该时间点一次性生成的粒子数")));

		ZCOMPONENT(ParticleSystemPreviewComponent, ZPROP(Inherent), ZPROP(HideComponentHeader))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, IsLocalSpace, SetLocalSpace, IsLocalSpace, ZPROP(Category, ZTEXT("Emitter")), ZPROP(Tooltip, ZTEXT("是否在局部空间模拟粒子")))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, LoopCount, SetLoopCount, GetLoopCount, ZPROP(Category, ZTEXT("Emitter")), ZPROP(Tooltip, ZTEXT("循环次数。若小于等于0, 则为无限循环")))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, LoopDuration, SetLoopDuration, GetLoopDuration, ZPROP(Category, ZTEXT("Emitter")), ZPROP(Tooltip, ZTEXT("每次循环的时长")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, SpawnRate, GetSpawnRate, ZPROP(Category, ZTEXT("Emitter")), ZPROP(Tooltip, ZTEXT("每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, BurstList, GetBurstList, ZPROP(Category, ZTEXT("Emitter")), ZPROP(Tooltip, ZTEXT("每个时间点一次性生成的粒子数列表")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, InitialPosition, GetInitialPosition, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的初始位置")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, InitialRotation, GetInitialRotation, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的初始旋转")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, RotationRate, GetRotationRate, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的旋转速度")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, SizeBegin, GetSizeBegin, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的初始大小")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, SizeEnd, GetSizeEnd, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的最终大小")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, InitialVelocity, GetInitialVelocity, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的初始速度")))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, InheritVelocityRatio, SetInheritVelocityRatio, GetInheritVelocityRatio, ZPROP(DisplayName, ZTEXT("InheritVelocity")), ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("决定粒子所能从发射器继承的速度比率。该变量只有在非局部空间有效")), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, ColorBegin, GetColorBegin, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的初始颜色")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, ColorEnd, GetColorEnd, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的最终颜色")))
			ZDATA_GETTER_REF(ParticleSystemPreviewComponent, Lifetime, GetLifetime, ZPROP(Category, ZTEXT("Particle")), ZPROP(Tooltip, ZTEXT("粒子的生命周期")))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, Texture, SetTexture, GetTexture, ZPROP(Category, ZTEXT("Renderer")), ZPROP(Tooltip, ZTEXT("粒子的材质贴图")))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, SubImageSize, SetSubImageSize, GetSubImageSize, ZPROP(Category, ZTEXT("Renderer")), ZPROP(Tooltip, ZTEXT("决定如何分割贴图来用于UV动画。x为列数，y为行数")), ZPROP(ClampMin, 0.0f))
			ZDATA_SETTER_GETTER(ParticleSystemPreviewComponent, MaxParticles, SetMaxParticles, GetMaxParticles, ZPROP(Category, ZTEXT("Emitter")), ZPROP(Tooltip, ZTEXT("最多生成的粒子数")), ZPROP(ClampMin, 0));

		ZENUM(Rigidbody2DComponent::BodyType)
			ZENUM_DATA(Rigidbody2DComponent::BodyType, Static, ZPROP(Tooltip, ZTEXT("静态")))
			ZENUM_DATA(Rigidbody2DComponent::BodyType, Dynamic, ZPROP(Tooltip, ZTEXT("动态")))
			ZENUM_DATA(Rigidbody2DComponent::BodyType, Kinematic, ZPROP(Tooltip, ZTEXT("运动学")));

		ZCOMPONENT(Rigidbody2DComponent, ZPROP(DisplayName, ZTEXT("Rigidbody 2D")), ZPROP(Tooltip, ZTEXT("2D刚体组件")), ZPROP(Category, ZTEXT("Physics")))
			ZDATA(Rigidbody2DComponent, Type, ZPROP(Tooltip, ZTEXT("刚体类型")))
			ZDATA(Rigidbody2DComponent, bFixedRotation);

		ZCOMPONENT(BoxCollider2DComponent, ZPROP(DisplayName, ZTEXT("Box Collider 2D")), ZPROP(Tooltip, ZTEXT("2D碰撞盒组件")), ZPROP(Category, ZTEXT("Physics")))
			ZDATA(BoxCollider2DComponent, Offset)
			ZDATA(BoxCollider2DComponent, Size, ZPROP(Tooltip, ZTEXT("碰撞大小，该值为缩放倍数")))
			ZDATA(BoxCollider2DComponent, Density, ZPROP(Tooltip, ZTEXT("密度")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(BoxCollider2DComponent, Friction, ZPROP(Tooltip, ZTEXT("摩擦力")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(BoxCollider2DComponent, Restitution, ZPROP(Tooltip, ZTEXT("弹力")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(BoxCollider2DComponent, RestitutionThreshold, ZPROP(Tooltip, ZTEXT("弹力阈值，速度高于该值时的碰撞将会反弹")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f));

		ZCOMPONENT(CircleCollider2DComponent, ZPROP(DisplayName, ZTEXT("Circle Collider 2D")), ZPROP(Tooltip, ZTEXT("2D碰撞圆组件")), ZPROP(Category, ZTEXT("Physics")))
			ZDATA(CircleCollider2DComponent, Offset)
			ZDATA(CircleCollider2DComponent, Radius, ZPROP(Tooltip, ZTEXT("碰撞半径")))
			ZDATA(CircleCollider2DComponent, Density, ZPROP(Tooltip, ZTEXT("密度")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(CircleCollider2DComponent, Friction, ZPROP(Tooltip, ZTEXT("摩擦力")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(CircleCollider2DComponent, Restitution, ZPROP(Tooltip, ZTEXT("弹力")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f))
			ZDATA(CircleCollider2DComponent, RestitutionThreshold, ZPROP(Tooltip, ZTEXT("弹力阈值，速度高于该值时的碰撞将会反弹")), ZPROP(Category, ZTEXT("Physics Material")), ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f));

		ZCOMPONENT(MeshRendererComponent, ZPROP(DisplayName, ZTEXT("Mesh Renderer")), ZPROP(Tooltip, ZTEXT("网格渲染组件")), ZPROP(Category, ZTEXT("Rendering")))
			ZDATA(MeshRendererComponent, MeshAsset)
			ZDATA_GETTER_REF(MeshRendererComponent, MaterialSlots, GetMaterials, ZPROP(FixedSizeContainer), ZPROP(CustomElementName));

		ZCOMPONENT(MeshPreviewComponent, ZPROP(Inherent), ZPROP(HideComponentHeader))
			ZDATA_GETTER_REF(MeshPreviewComponent, MaterialSlots, GetMaterials, ZPROP(FixedSizeContainer), ZPROP(CustomElementName));

		ZENUM(LightComponent::LightType)
			ZENUM_DATA(LightComponent::LightType, DirectionalLight, ZPROP(Tooltip, ZTEXT("平行光")))
			ZENUM_DATA(LightComponent::LightType, PointLight, ZPROP(Tooltip, ZTEXT("点光源")))
			ZENUM_DATA(LightComponent::LightType, SpotLight, ZPROP(Tooltip, ZTEXT("聚光灯")));

		ZENUM(Light::ShadowType)
			ZENUM_DATA(Light::ShadowType, HardShadow, ZPROP(Tooltip, ZTEXT("硬阴影")))
			ZENUM_DATA(Light::ShadowType, PCF, ZPROP(Tooltip, ZTEXT("PCF软阴影")))
			ZENUM_DATA(Light::ShadowType, PCSS, ZPROP(Tooltip, ZTEXT("PCSS软阴影")));

		ZCOMPONENT(LightComponent, ZPROP(DisplayName, ZTEXT("Light")), ZPROP(Tooltip, ZTEXT("灯光组件")), ZPROP(Category, ZTEXT("Rendering")))
			ZDATA(LightComponent, Type)
			ZDATA_SETTER_GETTER(LightComponent, Color, SetColor, GetColor)
			ZDATA_SETTER_GETTER(LightComponent, Intensity, SetIntensity, GetIntensity, ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f))
			ZDATA_SETTER_GETTER(LightComponent, Range, SetRange, GetRange, ZPROP(DragSensitivity, 0.1f), ZPROP(ClampMin, 0.0f), ZPROP(HideCondition, "Type == DirectionalLight"))
			ZDATA_SETTER_GETTER(LightComponent, CutoffAngle, SetCutoffToRadians, GetCutoffAsDegrees, ZPROP(DragSensitivity, 0.1f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 89.0f), ZPROP(HideCondition, "Type != SpotLight"))
			ZDATA_SETTER_GETTER(LightComponent, CastShadow, SetCastShadow, IsCastShadow)
			ZDATA_SETTER_GETTER(LightComponent, ShadowType, SetShadowType, GetShadowType, ZPROP(HideCondition, "CastShadow == false"))
			ZDATA_SETTER_GETTER(LightComponent, DepthBias, SetDepthBias, GetDepthBias, ZPROP(DragSensitivity, 0.001f), ZPROP(ClampMin, 0.0f), ZPROP(HideCondition, "CastShadow == false"))
			ZDATA_SETTER_GETTER(LightComponent, NormalBias, SetNormalBias, GetNormalBias, ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(HideCondition, "CastShadow == false"))
			ZDATA_SETTER_GETTER(LightComponent, FilterSize, SetFilterSize, GetFilterSize, ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			ZDATA_SETTER_GETTER(LightComponent, LightSize, SetLightSize, GetLightSize, ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(HideCondition, "CastShadow == false || ShadowType != PCSS"))
			ZDATA_SETTER_GETTER(LightComponent, CascadeCount, SetCascadeCount, GetCascadeCount, ZPROP(DragSensitivity, 0.1f), ZPROP(ClampMin, 1), ZPROP(ClampMax, 4), ZPROP(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			ZDATA_SETTER_GETTER(LightComponent, CascadeBlendThreshold, SetCascadeBlendThreshold, GetCascadeBlendThreshold, ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f), ZPROP(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			ZDATA_SETTER_GETTER(LightComponent, MaxShadowDistance, SetMaxShadowDistance, GetMaxShadowDistance, ZPROP(DragSensitivity, 1.0f), ZPROP(ClampMin, 0.0f), ZPROP(HideCondition, "CastShadow == false || Type != DirectionalLight"))
			ZDATA_SETTER_GETTER(LightComponent, CascadeSplitLambda, SetCascadeSplitLambda, GetCascadeSplitLambda, ZPROP(DragSensitivity, 0.01f), ZPROP(ClampMin, 0.0f), ZPROP(ClampMax, 1.0f), ZPROP(HideCondition, "CastShadow == false || Type != DirectionalLight"));

		ZCOMPONENT(MaterialPreviewComponent, ZPROP(Inherent), ZPROP(HideComponentHeader))
			ZDATA_SETTER_GETTER(MaterialPreviewComponent, Shader, SetShader, GetShader);

		ZCOMPONENT(BillboardComponent, ZPROP(Inherent), ZPROP(Transient));

		ZCOMPONENT(BoundsComponent, ZPROP(Inherent), ZPROP(Transient));

		ZENUM(SamplerType)
			ZENUM_DATA(SamplerType, BilinearRepeat)
			ZENUM_DATA(SamplerType, BilinearClamp)
			ZENUM_DATA(SamplerType, PointRepeat)
			ZENUM_DATA(SamplerType, PointClamp);

		ZCOMPONENT(TexturePreviewComponent, ZPROP(Inherent), ZPROP(HideComponentHeader))
			ZDATA_SETTER_GETTER(TexturePreviewComponent, SRGB, SetSRGB, IsSRGB)
			ZDATA_SETTER_GETTER(TexturePreviewComponent, SamplerType, SetSamplerType, GetSamplerType)
			ZDATA_SETTER_GETTER(TexturePreviewComponent, GenerateMipmaps, SetGenerateMipmaps, ShouldGenerateMipmaps);

	}

}
