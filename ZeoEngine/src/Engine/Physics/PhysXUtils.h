#pragma once

#include <foundation/PxVec4.h>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class PhysXUtils
	{
	public:
		static const physx::PxVec3& ToPhysXVector(const Vec3& v);
		static const physx::PxVec4& ToPhysXVector(const Vec4& v);
	};
	
}
