#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include "Engine/Renderer/RenderPass.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Math/Math.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	Vec3 Light::CalculateDirection(const Vec3& rotation) const
	{
		return Math::GetForwardVector(rotation);
	}

	void DirectionalLight::OnShadowTypeChanged() const
	{
		const auto* shadowPass = EngineUtils::GetRenderPassFromContext<ScreenSpaceShadowPass>(GetScene(), "ScreenSpaceShadow");
		const auto shadowShader = shadowPass->GetShadowShader();
		shadowShader->SetShaderVariantByMacro("SHADOW_TYPE", static_cast<U32>(GetShadowType()));
	}

}
