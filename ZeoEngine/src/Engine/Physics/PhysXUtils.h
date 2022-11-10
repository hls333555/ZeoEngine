#pragma once

#include <foundation/PxVec4.h>
#include <foundation/PxTransform.h>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class PhysXUtils
	{
	public:
		static const physx::PxVec3& ToPhysXVector(const Vec3& vector);
		static const physx::PxVec4& ToPhysXVector(const Vec4& vector);
		static physx::PxTransform ToPhysXTransform(const Mat4& transform);
		static physx::PxTransform ToPhysXTransform(const Vec3& translation, const Vec3& rotation);
		static physx::PxQuat ToPhysXQuat(const Quat& quat);

		static Vec3 FromPhysXVector(const physx::PxVec3& vector);
		static Vec4 FromPhysXVector(const physx::PxVec4& vector);
		static Mat4 FromPhysXTransform(const physx::PxTransform& transform);
		static Quat FromPhysXQuat(const physx::PxQuat& quat);

	};
	
}
