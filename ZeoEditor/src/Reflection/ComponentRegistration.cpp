#include "Engine/GameFramework/Components.h"
#include "Reflection/ReflectionHelper.h"

ZE_REFL_REGISTRATION
{
	using namespace ZeoEngine;

	ZE_REFL_TYPE(TagComponent, ZE_REFL_PROP_PAIR(Tooltip, u8"标签组件"))
		ZE_REFL_DATA_REF(TagComponent, Tag)
#if TEST
		ZE_REFL_DATA_REF(TagComponent, ui8var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint8_t))
		ZE_REFL_DATA_REF(TagComponent, ui32var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint32_t))
		ZE_REFL_DATA_REF(TagComponent, ui64var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, uint64_t))
		ZE_REFL_DATA_REF(TagComponent, i8var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int8_t))
		ZE_REFL_DATA_REF(TagComponent, i32var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int32_t))
		ZE_REFL_DATA_REF(TagComponent, i64var, ZE_REFL_PROP_PAIR_WITH_CAST(ClampMin, 1, int64_t))
		ZE_REFL_DATA_REF(TagComponent, floatvar, ZE_REFL_PROP_PAIR(ClampMin, 1.0f))
		ZE_REFL_DATA_REF(TagComponent, doublevar, ZE_REFL_PROP_PAIR(ClampMin, 1.0))
#endif
;

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
