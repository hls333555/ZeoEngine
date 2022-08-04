#include "ZEpch.h"
#include "Engine/Renderer/Light.h"

#include "Engine/Math/Math.h"

namespace ZeoEngine {

	Vec3 Light::CalculateDirection(const Vec3& rotation) const
	{
		return Math::GetForwardVector(rotation);
	}

}
