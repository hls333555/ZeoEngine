#include "ZEpch.h"
#include "Engine/Physics/PhysXShapes.h"

#include <extensions/PxRigidActorExt.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXActor.h"
#include "Engine/Physics/PhysXUtils.h"
#include "Engine/Physics/CollisionLayer.h"

namespace ZeoEngine {

	void PhysXColliderShapeBase::CreateShape(Entity entity, const PhysXActor& actor, const physx::PxGeometry& geometry, const ColliderComponentBase& colliderComp, U32 collisionDetectionType, const Mat4& transform)
	{
		m_Entity = entity;

		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, *PhysXUtils::ToPhysXMaterial(colliderComp.PhysicsMaterialAsset));
		SetTransform(transform);
		if (colliderComp.bIsTrigger)
		{
			m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		}
		else
		{
			m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, colliderComp.bEnableSimulation);
		}
		m_Shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, colliderComp.bEnableQuery);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, colliderComp.bIsTrigger);

		const U32 collisionLayerBit = ZE_BIT(colliderComp.CollisionLayer);
		const U32 collisionGroupBits = CollisionLayerManager::GetGroupLayerMask(colliderComp.CollidesWithGroup);
		physx::PxFilterData simulationFilterData;
		simulationFilterData.word0 = collisionLayerBit;
		simulationFilterData.word1 = collisionGroupBits;
		simulationFilterData.word2 = collisionDetectionType;
		m_Shape->setSimulationFilterData(simulationFilterData);
		physx::PxFilterData queryFilterData;
		queryFilterData.word0 = collisionLayerBit;
		m_Shape->setQueryFilterData(queryFilterData);

		m_Shape->userData = this;
	}

	bool PhysXColliderShapeBase::IsSimulationEnabled() const
	{
		return m_Shape->getFlags().isSet(physx::PxShapeFlag::eSIMULATION_SHAPE);
	}

	void PhysXColliderShapeBase::SetSimulationEnabled(bool bEnable) const
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, bEnable);
	}

	bool PhysXColliderShapeBase::IsQueryEnabled() const
	{
		return m_Shape->getFlags().isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
	}

	void PhysXColliderShapeBase::SetQueryEnabled(bool bEnable) const
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, bEnable);
	}

	void PhysXColliderShapeBase::DetachFromActor(physx::PxRigidActor* actor) const
	{
		actor->detachShape(*m_Shape);
	}

	void PhysXColliderShapeBase::SetTransform(const Mat4& transform) const
	{
		const auto pose = PhysXUtils::ToPhysXTransform(GetOffsetTransform(transform));
		m_Shape->setLocalPose(pose);
	}

	PhysXBoxColliderShape::PhysXBoxColliderShape(Entity entity, const PhysXActor& actor, U32 collisionDetectionType, const Mat4& transform)
	{
		const auto& boxComp = entity.GetComponent<BoxColliderComponent>();

		const Vec3& scale = entity.GetWorldScale();
		const Vec3 scaledSize = scale * boxComp.Size;
		const auto box = physx::PxBoxGeometry(scaledSize.x * 0.5f, scaledSize.y * 0.5f, scaledSize.z * 0.5f);
		CreateShape(entity, actor, box, boxComp, collisionDetectionType, transform);
	}

	Mat4 PhysXBoxColliderShape::GetOffsetTransform(const Mat4& transform) const
	{
		const auto& boxComp = GetEntity().GetComponent<BoxColliderComponent>();
		return glm::translate(transform, boxComp.Offset);
	}

	PhysXSphereColliderShape::PhysXSphereColliderShape(Entity entity, const PhysXActor& actor, U32 collisionDetectionType, const Mat4& transform)
	{
		const auto& sphereComp = entity.GetComponent<SphereColliderComponent>();

		const Vec3& scale = entity.GetWorldScale();
		const float largestScale = glm::max(scale.x, glm::max(scale.y, scale.z));
		const auto sphere = physx::PxSphereGeometry(sphereComp.Radius * largestScale);
		CreateShape(entity, actor, sphere, sphereComp, collisionDetectionType, transform);
	}

	Mat4 PhysXSphereColliderShape::GetOffsetTransform(const Mat4& transform) const
	{
		const auto& sphereComp = GetEntity().GetComponent<SphereColliderComponent>();
		return glm::translate(transform, sphereComp.Offset);
	}

	PhysXCapsuleColliderShape::PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor, U32 collisionDetectionType, const Mat4& transform)
	{
		const auto& capsuleComp = entity.GetComponent<CapsuleColliderComponent>();

		const Vec3& scale = entity.GetWorldScale();
		const float radiusScale = glm::max(scale.x, scale.z);
		const auto capsule = physx::PxCapsuleGeometry(capsuleComp.Radius * radiusScale, (capsuleComp.Height * 0.5f) * scale.y);
		CreateShape(entity, actor, capsule, capsuleComp, collisionDetectionType, transform);
	}

	Mat4 PhysXCapsuleColliderShape::GetOffsetTransform(const Mat4& transform) const
	{
		const auto& capsuleComp = GetEntity().GetComponent<CapsuleColliderComponent>();
		return glm::translate(transform, capsuleComp.Offset) * glm::toMat4(Quat(Vec3(0.0f, 0.0f, physx::PxHalfPi/* PhysX's capsule is horizontal */)));
	}

}
