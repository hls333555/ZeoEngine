#include "Engine/GameFramework/Components.h"
#include "Reflection/ReflectionHelper.h"

ZE_REFL_REGISTRATION
{
	using namespace ZeoEngine;

#if ENABLE_TEST
	ZE_REFL_TYPE(TestComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"测试组件"))
		ZE_REFL_DATA_REF(TestComponent, BoolVar)
		ZE_REFL_DATA_REF(TestComponent, Ui8Var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint8_t))
		ZE_REFL_DATA_REF(TestComponent, Ui32Var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint32_t))
		ZE_REFL_DATA_REF(TestComponent, Ui64Var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint64_t))
		ZE_REFL_DATA_REF(TestComponent, I8Var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int8_t))
		ZE_REFL_DATA_REF(TestComponent, I32Var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int32_t))
		ZE_REFL_DATA_REF(TestComponent, I64Var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int64_t))
		ZE_REFL_DATA_REF(TestComponent, FloatVar, ZE_REFL_PROP_PAIR(ClampMin, 1.0f))
		ZE_REFL_DATA_REF(TestComponent, DoubleVar, ZE_REFL_PROP_PAIR(ClampMin, 1.0))
		ZE_REFL_DATA_REF(TestComponent, StringVar)
		ZE_REFL_DATA_REF(TestComponent, Vec2Var, ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA_REF(TestComponent, Vec3Var, ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
		ZE_REFL_DATA_REF(TestComponent, ColorVar);
#endif

	ZE_REFL_TYPE(TagComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"标签组件"))
		ZE_REFL_DATA_REF(TagComponent, Tag);

	ZE_REFL_TYPE(TransformComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"变换组件"))
		ZE_REFL_DATA_REF(TransformComponent, Transform);

	ZE_REFL_TYPE(SpriteRendererComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"2D渲染组件"))
		ZE_REFL_DATA_REF(SpriteRendererComponent, TintColor)
		ZE_REFL_DATA_REF(SpriteRendererComponent, Texture)
		ZE_REFL_DATA_REF(SpriteRendererComponent, TextureTiling);

	ZE_REFL_TYPE(CameraComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"相机组件"))
		ZE_REFL_DATA_REF(CameraComponent, bIsPrimary)
		ZE_REFL_DATA_REF(CameraComponent, bFixedAspectRatio);

	ZE_REFL_TYPE(NativeScriptComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"C++脚本组件"));

}
