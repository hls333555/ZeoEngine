#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionCore.h"

ZE_REFL_REGISTRATION
{
using namespace ZeoEngine;

#if ENABLE_TEST_COMPONENT
	ZE_REFL_ENUM(TestComponent::TestEnum)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum1)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum2)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum3);

	ZE_REFL_ENUM(TestComponent::TestEnumClass)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass1)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass2)
		ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass3);

	ZE_REFL_STRUCT(TestComponent::TestStruct1)
		ZE_REFL_DATA(TestComponent::TestStruct1, EnumVar)
		ZE_REFL_DATA(TestComponent::TestStruct1, I32Var);

	ZE_REFL_STRUCT(TestComponent::TestStruct2)
		ZE_REFL_DATA(TestComponent::TestStruct2, TestStruct1Var)
		ZE_REFL_DATA(TestComponent::TestStruct2, FloatVar);

	ZE_REFL_COMPONENT(TestComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Test")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("测试组件")), ZE_REFL_PROP(Inherent))
		ZE_REFL_DATA(TestComponent, BoolVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, Ui8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA(TestComponent, Ui32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA(TestComponent, Ui64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
		ZE_REFL_DATA(TestComponent, I8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
		ZE_REFL_DATA(TestComponent, I32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
		ZE_REFL_DATA(TestComponent, I64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
		ZE_REFL_DATA(TestComponent, FloatVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging))
		ZE_REFL_DATA(TestComponent, DoubleVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1.0))
		ZE_REFL_DATA(TestComponent, EnumVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, EnumClassVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, StringVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, Vec2Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA(TestComponent, Vec3Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA(TestComponent, ColorVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, Texture2DVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, ParticleTemplateVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, TestStruct1Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_GETTER_REF(TestComponent, TestStruct2GetterVar, GetTestStruct2GetterVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA_SETTER_GETTER(TestComponent, ShowSequenceContainers, SetShowSequenceContainers, GetShowSequenceContainers, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
		ZE_REFL_DATA(TestComponent, BoolVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, Ui8VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, Ui32VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, Ui64VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, I8VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(ClampMin, 1), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, I32VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, I64VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, FloatVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, DoubleVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, EnumVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, StringVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, Vec2VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, Vec3VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, ColorVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, Texture2DVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, ParticleTemplateVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA(TestComponent, TestStruct1VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
		ZE_REFL_DATA_GETTER_REF(TestComponent, TestStruct2VecGetterVar, GetTestStruct2VecGetterVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"));
#endif

	ZE_REFL_COMPONENT(CoreComponent, ZE_REFL_PROP(Inherent), ZE_REFL_PROP(HideComponentHeader))
		ZE_REFL_DATA(CoreComponent, Name)
		ZE_REFL_DATA(CoreComponent, bIsInternal, ZE_REFL_PROP(HiddenInEditor));

	ZE_REFL_COMPONENT(TransformComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Transform")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("变换组件")), ZE_REFL_PROP(Inherent))
		ZE_REFL_DATA(TransformComponent, Translation, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("平移")), ZE_REFL_PROP_PAIR(DragSensitivity, 0.1f))
		ZE_REFL_DATA_SETTER_GETTER(TransformComponent, Rotation, SetRotationToRadians, GetRotationAsDegrees, ZE_REFL_PROP_PAIR(ClampMin, -360.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("旋转")))
		ZE_REFL_DATA(TransformComponent, Scale, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("缩放")), ZE_REFL_PROP_PAIR(DragSensitivity, 0.1f));

	ZE_REFL_COMPONENT(SpriteRendererComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Sprite Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("2D渲染组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Rendering")))
		ZE_REFL_DATA(SpriteRendererComponent, TintColor)
		ZE_REFL_DATA(SpriteRendererComponent, Texture)
		ZE_REFL_DATA(SpriteRendererComponent, TextureTiling)
		ZE_REFL_DATA(SpriteRendererComponent, SortingOrder);

	ZE_REFL_ENUM(SceneCamera::ProjectionType)
		ZE_REFL_ENUM_DATA(SceneCamera::ProjectionType, Perspective, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("透视相机")))
		ZE_REFL_ENUM_DATA(SceneCamera::ProjectionType, Orthographic, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("正交相机")));

	ZE_REFL_COMPONENT(CameraComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Camera")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("相机组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Rendering")))
		ZE_REFL_DATA(CameraComponent, bIsPrimary)
		ZE_REFL_DATA(CameraComponent, bFixedAspectRatio)
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, ProjectionType, SetProjectionType, GetProjectionType)
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, VerticalFOV, SetPerspectiveVerticalFOV, GetPerspectiveVerticalFOV, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, PerspectiveNear, SetPerspectiveNearClip, GetPerspectiveNearClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, PerspectiveFar, SetPerspectiveFarClip, GetPerspectiveFarClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Size, SetOrthographicSize, GetOrthographicSize, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, OrthographicNear, SetOrthographicNearClip, GetOrthographicNearClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, OrthographicFar, SetOrthographicFarClip, GetOrthographicFarClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"));

	ZE_REFL_COMPONENT(NativeScriptComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Native Script")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("C++脚本组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Scripts")));

	ZE_REFL_COMPONENT(ParticleSystemComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Particle System")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子系统组件")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Effects")))
		ZE_REFL_DATA(ParticleSystemComponent, Template)
		ZE_REFL_DATA(ParticleSystemComponent, PositionOffset);

	ZE_REFL_ENUM(ParticleVariationType)
		ZE_REFL_ENUM_DATA(ParticleVariationType, Constant)
		ZE_REFL_ENUM_DATA(ParticleVariationType, RandomInRange)
		ZE_REFL_ENUM_DATA(ParticleVariationType, UniformInRange);

#define ZE_REFL_PARTICLE_VARIATION_TYPE(_type)										\
	ZE_REFL_STRUCT(_type)															\
		ZE_REFL_DATA(_type, VariationType)											\
		ZE_REFL_DATA(_type, Val1, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Value")))	\
		ZE_REFL_DATA(_type, Val2, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("ValueHigh")), ZE_REFL_PROP_PAIR(HideCondition, "VariationType == Constant"))

	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleInt);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleFloat);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleVec2);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleVec3);
	ZE_REFL_PARTICLE_VARIATION_TYPE(ParticleColor);

	ZE_REFL_STRUCT(BurstData)
		ZE_REFL_DATA(BurstData, Time, ZE_REFL_PROP_PAIR(ClampMin, 0.0f), ZE_REFL_PROP_PAIR(ClampMax, 1.0f), ZE_REFL_PROP_PAIR(DragSensitivity, 0.01f), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("标准化的时间点：[0,1]")))
		ZE_REFL_DATA(BurstData, Amount, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("在该时间点一次性生成的粒子数")));

	ZE_REFL_COMPONENT(ParticleSystemPreviewComponent, ZE_REFL_PROP(Inherent), ZE_REFL_PROP(HideComponentHeader))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, IsLocalSpace, SetLocalSpace, IsLocalSpace, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("是否在局部空间模拟粒子")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, LoopCount, SetLoopCount, GetLoopCount, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("循环次数。若小于等于0, 则为无限循环")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, LoopDuration, SetLoopDuration, GetLoopDuration, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("每次循环的时长")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, SpawnRate, GetSpawnRate, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, BurstList, GetBurstList, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("每个时间点一次性生成的粒子数列表")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, InitialPosition, GetInitialPosition, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始位置")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, InitialRotation, GetInitialRotation, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始旋转")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, RotationRate, GetRotationRate, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的旋转速度")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, SizeBegin, GetSizeBegin, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始大小")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, SizeEnd, GetSizeEnd, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的最终大小")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, InitialVelocity, GetInitialVelocity, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始速度")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, InheritVelocityRatio, SetInheritVelocityRatio, GetInheritVelocityRatio, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("InheritVelocity")), ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("决定粒子所能从发射器继承的速度比率。该变量只有在非局部空间有效")), ZE_REFL_PROP_PAIR(ClampMin, 0.0f), ZE_REFL_PROP_PAIR(ClampMax, 1.0f))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, ColorBegin, GetColorBegin, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的初始颜色")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, ColorEnd, GetColorEnd, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的最终颜色")))
		ZE_REFL_DATA_GETTER_REF(ParticleSystemPreviewComponent, Lifetime, GetLifetime, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Particle")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的生命周期")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, Texture, SetTexture, GetTexture, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子的材质贴图")))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, SubImageSize, SetSubImageSize, GetSubImageSize, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("决定如何分割贴图来用于UV动画。x为列数，y为行数")), ZE_REFL_PROP_PAIR(ClampMin, 0.0f))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, MaxParticles, SetMaxParticles, GetMaxParticles, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Emitter")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("最多生成的粒子数")), ZE_REFL_PROP_PAIR(ClampMin, 0))
		ZE_REFL_DATA_SETTER_GETTER(ParticleSystemPreviewComponent, PreviewThumbnail, SetPreviewThumbnail, GetPreviewThumbnail, ZE_REFL_PROP(HiddenInEditor));

}
