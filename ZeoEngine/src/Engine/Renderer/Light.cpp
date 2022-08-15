#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include "Engine/Math/Math.h"

namespace ZeoEngine {

	Vec3 Light::CalculateDirection(const Vec3& rotation) const
	{
		return Math::GetForwardVector(rotation);
	}

	void DirectionalLight::SetShadowType(ShadowType type)
	{
		Light::SetShadowType(type);

		ScreenSpaceShadowPass::GetShadowShader()->SetShaderVariantByMacro("SHADOW_TYPE", static_cast<U32>(type));
	}
}
