#include "Engine/GameFramework/Components.h"
#include "Reflection/RegistrationHelper.h"

ZE_REFL_REGISTRATION
{
	using namespace ZeoEngine;

	ZE_REFL_COMP(TransformComponent)
		ZE_REFL_DATA_REF(TransformComponent, Transform);

	ZE_REFL_COMP(TagComponent)
		ZE_REFL_DATA_REF(TagComponent, Tag);

	ZE_REFL_COMP(SpriteRendererComponent)
		ZE_REFL_DATA_REF(SpriteRendererComponent, TintColor)
		ZE_REFL_DATA_REF(SpriteRendererComponent, Texture)
		ZE_REFL_DATA_REF(SpriteRendererComponent, TextureTiling);

	ZE_REFL_COMP(CameraComponent)
		ZE_REFL_DATA(CameraComponent, bIsPrimary)
		ZE_REFL_DATA(CameraComponent, bFixedAspectRatio);

	ZE_REFL_COMP(NativeScriptComponent);

}
