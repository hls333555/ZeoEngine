#include "ZEpch.h"
#include "Engine/Physics/PhysXUtils.h"

#include <glm/gtx/quaternion.hpp>

#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/PhysicsMaterial.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Math/Math.h"

namespace ZeoEngine {

	const physx::PxVec3& PhysXUtils::ToPhysXVector(const Vec3& vector)
	{
		return *reinterpret_cast<const physx::PxVec3*>(&vector);
	}

	const physx::PxVec4& PhysXUtils::ToPhysXVector(const Vec4& vector)
	{
		return *reinterpret_cast<const physx::PxVec4*>(&vector);
	}

	physx::PxExtendedVec3 PhysXUtils::ToPhysXExtendedVector(const Vec3& vector)
	{
		return { vector.x, vector.y, vector.z };
	}

	physx::PxTransform PhysXUtils::ToPhysXTransform(const Mat4& transform)
	{
		glm::vec3 translation, rotation, scale;
		Math::DecomposeTransform(transform, translation, rotation, scale);

		physx::PxQuat quat = ToPhysXQuat(Quat(rotation));
		physx::PxVec3 pos = ToPhysXVector(translation);
		return { pos, quat };
	}

	physx::PxTransform PhysXUtils::ToPhysXTransform(const Vec3& translation, const Vec3& rotation)
	{
		return { ToPhysXVector(translation), ToPhysXQuat(Quat(rotation)) };
	}

	physx::PxQuat PhysXUtils::ToPhysXQuat(const Quat& quat)
	{
		return { quat.x, quat.y, quat.z, quat.w };
	}

	Vec3 PhysXUtils::FromPhysXVector(const physx::PxVec3& vector)
	{
		return *reinterpret_cast<const Vec3*>(&vector);
	}

	Vec4 PhysXUtils::FromPhysXVector(const physx::PxVec4& vector)
	{
		return *reinterpret_cast<const Vec4*>(&vector);
	}

	Vec3 PhysXUtils::FromPhysXExtendedVector(const physx::PxExtendedVec3& vector)
	{
		return { vector.x, vector.y, vector.z };
	}

	Mat4 PhysXUtils::FromPhysXTransform(const physx::PxTransform& transform)
	{
		const Quat rotation = FromPhysXQuat(transform.q);
		const Vec3 position = FromPhysXVector(transform.p);
		return glm::translate(Mat4(1.0f), position) * glm::toMat4(rotation);
	}

	Quat PhysXUtils::FromPhysXQuat(const physx::PxQuat& quat)
	{
		return *reinterpret_cast<const Quat*>(&quat);
	}

	physx::PxMaterial* PhysXUtils::ToPhysXMaterial(AssetHandle physicsMaterialAsset)
	{
		const auto physicsMaterial = AssetLibrary::LoadAsset<PhysicsMaterial>(physicsMaterialAsset);
		if (physicsMaterial)
		{
			return physicsMaterial->m_PhysicsMaterial;
		}
		else
		{
			const auto defaultPhysicsMaterial = PhysXEngine::GetDefaultPhysicsMaterial();
			ZE_CORE_ASSERT(defaultPhysicsMaterial);
			return defaultPhysicsMaterial->m_PhysicsMaterial;
		}
	}

}
