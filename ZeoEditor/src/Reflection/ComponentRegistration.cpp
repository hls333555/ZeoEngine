#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionHelper.h"

ZE_REFL_REGISTRATION
{
using namespace ZeoEngine;

#if ENABLE_TEST
	ZE_REFL_ENUM(TestComponent::TestEnum)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum1)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum2)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum3);

	ZE_REFL_ENUM(TestComponent::TestEnumClass)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass1)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass2)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass3);

	ZE_REFL_TYPE(TestComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Test")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("测试组件")), ZE_REFL_PROP(InherentType))
		ZE_REFL_DATA_REF(TestComponent, BoolVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, Ui8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA_REF(TestComponent, Ui32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA_REF(TestComponent, Ui64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA_REF(TestComponent, I8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
		ZE_REFL_DATA_REF(TestComponent, I32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
		ZE_REFL_DATA_REF(TestComponent, I64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
		ZE_REFL_DATA_REF(TestComponent, FloatVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging))
		ZE_REFL_DATA_REF(TestComponent, DoubleVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1.0))
		ZE_REFL_DATA_REF(TestComponent, EnumVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, EnumClassVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, StringVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, Vec2Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA_REF(TestComponent, Vec3Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA_REF(TestComponent, ColorVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, Texture2DVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, ParticleTemplateVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_REF(TestComponent, BoolDeqVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, Ui8VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, Ui32VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, Ui64VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, I8VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA_REF(TestComponent, I32VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, I64VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, FloatVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, DoubleVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, EnumVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, StringVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, Vec2VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, Vec3VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, ColorVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")))
		ZE_REFL_DATA_REF(TestComponent, ParticleTemplateVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")));
#endif

	ZE_REFL_TYPE(CoreComponent, ZE_REFL_PROP(HideTypeHeader), ZE_REFL_PROP(InherentType))
		ZE_REFL_DATA_REF(CoreComponent, Name)
		ZE_REFL_DATA_REF(CoreComponent, bIsInternal, ZE_REFL_PROP(HiddenInEditor));

	ZE_REFL_TYPE(TransformComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Transform")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("变换组件")), ZE_REFL_PROP(InherentType))
		ZE_REFL_DATA_REF(TransformComponent, Translation, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("平移")), ZE_REFL_PROP_PAIR(DragSensitivity, 0.1f))
		ZE_REFL_DATA_SETTER_GETTER(TransformComponent, Rotation, SetRotationToRadians, GetRotationAsDegrees, ZE_REFL_PROP_PAIR(ClampMin, 0.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("旋转")))
		ZE_REFL_DATA_REF(TransformComponent, Scale, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("缩放")), ZE_REFL_PROP_PAIR(DragSensitivity, 0.1f));

	ZE_REFL_TYPE(SpriteRendererComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Sprite Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("2D渲染组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Rendering")))
		ZE_REFL_DATA_REF(SpriteRendererComponent, TintColor)
		ZE_REFL_DATA_REF(SpriteRendererComponent, Texture)
		ZE_REFL_DATA_REF(SpriteRendererComponent, TextureTiling);

	ZE_REFL_ENUM(SceneCamera::ProjectionType)
		ZE_REFL_ENUM_DATA(SceneCamera::ProjectionType, Perspective, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("透视相机")))
		ZE_REFL_ENUM_DATA(SceneCamera::ProjectionType, Orthographic, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("正交相机")));

	ZE_REFL_TYPE(CameraComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Camera")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("相机组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Rendering")))
		ZE_REFL_DATA_REF(CameraComponent, bIsPrimary)
		ZE_REFL_DATA_REF(CameraComponent, bFixedAspectRatio)
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, ProjectionType, SetProjectionType, GetProjectionType)
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, VerticalFOV, SetPerspectiveVerticalFOV, GetPerspectiveVerticalFOV, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, PerspectiveNear, SetPerspectiveNearClip, GetPerspectiveNearClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, PerspectiveFar, SetPerspectiveFarClip, GetPerspectiveFarClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Size, SetOrthographicSize, GetOrthographicSize, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, OrthographicNear, SetOrthographicNearClip, GetOrthographicNearClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, OrthographicFar, SetOrthographicFarClip, GetOrthographicFarClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"));

	ZE_REFL_TYPE(NativeScriptComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Native Script")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("C++脚本组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Scripts")));

	ZE_REFL_TYPE(ParticleSystemComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Particle System")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子系统组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Effects")))
		ZE_REFL_DATA_REF(ParticleSystemComponent, Template)
		ZE_REFL_DATA_REF(ParticleSystemComponent, PositionOffset);

	ZE_REFL_ENUM(ParticleVariationType)
		ZE_REFL_ENUM_DATA(ParticleVariationType, Constant)
		ZE_REFL_ENUM_DATA(ParticleVariationType, RandomInRange)
		ZE_REFL_ENUM_DATA(ParticleVariationType, UniformInRange);

#define ZE_REFL_PARTICLE_VARIATION_TYPE(_type)											\
	ZE_REFL_TYPE_NESTED(_type)															\
		ZE_REFL_DATA_REF(_type, VariationType)											\
		ZE_REFL_DATA_REF(_type, Val1, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Value")))	\
		ZE_REFL_DATA_REF(_type, Val2, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("ValueHigh")), ZE_REFL_PROP_PAIR(HideCondition, "VariationType == Constant"))

	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleInt);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleFloat);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleVec2);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleVec3);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleColor);

	ZE_REFL_TYPE_NESTED(BurstData)
		ZE_REFL_DATA_REF(BurstData, Time, ZE_REFL_PROP_PAIR(ClampMin, 0.0f), ZE_REFL_PROP_PAIR(ClampMax, 1.0f), ZE_REFL_PROP_PAIR(DragSensitivity, 0.01f), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("标准化的时间点：[0,1]")))
		ZE_REFL_DATA_REF(BurstData, Amount, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("在该时间点一次性生成的粒子数")));

	ZE_REFL_TYPE(ParticleSystemPreviewComponent, ZE_REFL_PROP(HideTypeHeader), ZE_REFL_PROP(InherentType))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, IsLocalSpace, SetLocalSpace, IsLocalSpace, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("是否在局部空间模拟粒子")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, LoopCount, SetLoopCount, GetLoopCount, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("循环次数。若小于等于0, 则为无限循环")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, LoopDuration, SetLoopDuration, GetLoopDuration, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("每次循环的时长")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, SpawnRate, SetSpawnRate, GetSpawnRate, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成")))
		ZE_REFL_DATA_SETTER_GETTER_REF(ParticleSystemPreviewComponent, BurstList, GetBurstList, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("每个时间点一次性生成的粒子数列表")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, InitialPosition, SetInitialPosition, GetInitialPosition, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始位置")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, InitialRotation, SetInitialRotation, GetInitialRotation, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始旋转")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, RotationRate, SetRotationRate, GetRotationRate, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的旋转速度")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, SizeBegin, SetSizeBegin, GetSizeBegin, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始大小")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, SizeEnd, SetSizeEnd, GetSizeEnd, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的最终大小")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, InitialVelocity, SetInitialVelocity, GetInitialVelocity, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始速度")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, InheritVelocity, SetInheritVelocity, GetInheritVelocity, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("决定粒子所能继承的速度。该变量只有在非局部空间有效")), ZE_REFL_PROP_PAIR(ClampMin, 0.0f), ZE_REFL_PROP_PAIR(ClampMax, 1.0f))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, ColorBegin, SetColorBegin, GetColorBegin, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始颜色")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, ColorEnd, SetColorEnd, GetColorEnd, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的最终颜色")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, Lifetime, SetLifetime, GetLifetime, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的生命周期")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, Texture, SetTexture, GetTexture, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的材质贴图")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, SubImageSize, SetSubImageSize, GetSubImageSize, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("决定如何分割贴图来用于UV动画。x为列数，y为行数")), ZE_REFL_PROP_PAIR(ClampMin, 0.0f))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, MaxDrawParticles, SetMaxDrawParticles, GetMaxDrawParticles, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("最多生成的粒子数")), ZE_REFL_PROP_PAIR(ClampMin, 0))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, PreviewThumbnail, SetPreviewThumbnail, GetPreviewThumbnail, ZE_REFL_PROP(HiddenInEditor));

}
