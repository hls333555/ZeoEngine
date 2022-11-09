#include "ZEpch.h"
#include "Engine/Physics/PhysXUtils.h"

namespace ZeoEngine {

	const physx::PxVec3& PhysXUtils::ToPhysXVector(const Vec3& v)
	{
		return *reinterpret_cast<const physx::PxVec3*>(&v);
	}

	const physx::PxVec4& PhysXUtils::ToPhysXVector(const Vec4& v)
	{
		return *reinterpret_cast<const physx::PxVec4*>(&v);
	}

}
