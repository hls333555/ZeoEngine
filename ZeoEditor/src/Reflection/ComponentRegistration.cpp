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

	ZE_REFL_TYPE(TestComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Test")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("测试组件")))
		ZE_REFL_DATA_REF(TestComponent, BoolVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")))
		ZE_REFL_DATA_REF(TestComponent, Ui8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")), ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint8_t))
		ZE_REFL_DATA_REF(TestComponent, Ui32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")), ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint32_t))
		ZE_REFL_DATA_REF(TestComponent, Ui64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")), ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint64_t))
		ZE_REFL_DATA_REF(TestComponent, I8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")), ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int8_t))
		ZE_REFL_DATA_REF(TestComponent, I32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")), ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int32_t))
		ZE_REFL_DATA_REF(TestComponent, I64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Integral")), ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int64_t))
		ZE_REFL_DATA_REF(TestComponent, FloatVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Floating Point")), ZE_REFL_PROP_PAIR(ClampMin, 1.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging))
		ZE_REFL_DATA_REF(TestComponent, DoubleVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Floating Point")), ZE_REFL_PROP_PAIR(ClampMin, 1.0))
		ZE_REFL_DATA_REF(TestComponent, enumVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Enum")))
		ZE_REFL_DATA_REF(TestComponent, enumClassVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Enum")))
		ZE_REFL_DATA_REF(TestComponent, StringVar)
		ZE_REFL_DATA_REF(TestComponent, Vec2Var, ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA_REF(TestComponent, Vec3Var, ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA_REF(TestComponent, ColorVar)
		ZE_REFL_DATA_REF(TestComponent, Texture2DVar);
#endif

	ZE_REFL_TYPE(CoreComponent, ZE_REFL_PROP(HideTypeHeader), ZE_REFL_PROP(InherentType))
		ZE_REFL_DATA_REF(CoreComponent, Name)
		ZE_REFL_DATA_REF(CoreComponent, bIsInternal, ZE_REFL_PROP(HiddenInEditor));

	ZE_REFL_TYPE(TransformComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Transform")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("变换组件")), ZE_REFL_PROP(InherentType))
		ZE_REFL_DATA_REF(TransformComponent, Translation, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("平移")))
		ZE_REFL_DATA_SETTER_GETTER(TransformComponent, Rotation, SetRotationToRadians, GetRotationAsDegrees, ZE_REFL_PROP_PAIR(ClampMin, 0.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("旋转")))
		ZE_REFL_DATA_REF(TransformComponent, Scale, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("缩放")));

	ZE_REFL_TYPE(SpriteRendererComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Sprite Renderer")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("2D渲染组件")))
		ZE_REFL_DATA_REF(SpriteRendererComponent, TintColor)
		ZE_REFL_DATA_REF(SpriteRendererComponent, Texture)
		ZE_REFL_DATA_REF(SpriteRendererComponent, TextureTiling);

	ZE_REFL_ENUM(SceneCamera::ProjectionType)
		ZE_REFL_ENUM_DATA(SceneCamera::ProjectionType, Perspective, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("透视相机")))
		ZE_REFL_ENUM_DATA(SceneCamera::ProjectionType, Orthographic, ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("正交相机")));

	ZE_REFL_TYPE(CameraComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Camera")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("相机组件")))
		ZE_REFL_DATA_REF(CameraComponent, bIsPrimary)
		ZE_REFL_DATA_REF(CameraComponent, bFixedAspectRatio)
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, ProjectionType, SetProjectionType, GetProjectionType)
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Vertical FOV, SetPerspectiveVerticalFOV, GetPerspectiveVerticalFOV, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Perspective Near, SetPerspectiveNearClip, GetPerspectiveNearClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Perspective Far, SetPerspectiveFarClip, GetPerspectiveFarClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Perspective")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Perspective"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Size, SetOrthographicSize, GetOrthographicSize, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Orthographic Near, SetOrthographicNearClip, GetOrthographicNearClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"))
		ZE_REFL_DATA_SETTER_GETTER(CameraComponent, Orthographic Far, SetOrthographicFarClip, GetOrthographicFarClip, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Orthographic")), ZE_REFL_PROP_PAIR(HideCondition, "ProjectionType != Orthographic"));

	ZE_REFL_TYPE(NativeScriptComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Native Script")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("C++脚本组件")));

	ZE_REFL_TYPE(ParticleSystemComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Particle System")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("粒子系统组件")))
		ZE_REFL_DATA_REF(ParticleSystemComponent, ParticleSystem);

	ZE_REFL_TYPE(ParticleSystemDetailComponent, ZE_REFL_PROP(HideTypeHeader));

}
