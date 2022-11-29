#include "ZEpch.h"
#include "Engine/Physics/PhysXShapes.h"

#include <extensions/PxRigidActorExt.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXActor.h"
#include "Engine/Physics/PhysXUtils.h"
#include "Engine/Physics/CollisionLayer.h"

namespace ZeoEngine {

	void PhysXColliderShapeBase::CreateShape(const PhysXActor& actor, const physx::PxGeometry& geometry, const ColliderComponentBase& colliderComp, const RigidBodyComponent& rigidBodyComp, const Vec3& translation, const Vec3& rotation)
	{
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, *PhysXUtils::ToPhysXMaterial(colliderComp.PhysicsMaterialAsset));
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(translation, rotation));
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
		simulationFilterData.word2 = static_cast<U32>(rigidBodyComp.CollisionDetection);
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

	PhysXBoxColliderShape::PhysXBoxColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& boxComp = entity.GetComponent<BoxColliderComponent>();
		const auto& rigidBodyComp = entity.GetComponent<RigidBodyComponent>();

		const Vec3 scaledSize = entity.GetScale() * boxComp.Size;
		const auto box = physx::PxBoxGeometry(scaledSize.x * 0.5f, scaledSize.y * 0.5f, scaledSize.z * 0.5f);
		CreateShape(actor, box, boxComp, rigidBodyComp, boxComp.Offset);
	}

	PhysXSphereColliderShape::PhysXSphereColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& sphereComp = entity.GetComponent<SphereColliderComponent>();
		const auto& rigidBodyComp = entity.GetComponent<RigidBodyComponent>();

		const auto& scale = entity.GetScale();
		const float largestScale = glm::max(scale.x, glm::max(scale.y, scale.z));
		const auto sphere = physx::PxSphereGeometry(sphereComp.Radius * largestScale);
		CreateShape(actor, sphere, sphereComp, rigidBodyComp, sphereComp.Offset);
	}

	PhysXCapsuleColliderShape::PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& capsuleComp = entity.GetComponent<CapsuleColliderComponent>();
		const auto& rigidBodyComp = entity.GetComponent<RigidBodyComponent>();

		const auto& scale = entity.GetScale();
		const float radiusScale = glm::max(scale.x, scale.z);
		const auto capsule = physx::PxCapsuleGeometry(capsuleComp.Radius * radiusScale, (capsuleComp.Height * 0.5f) * scale.y);
		CreateShape(actor, capsule, capsuleComp, rigidBodyComp, capsuleComp.Offset, Vec3(0.0f, 0.0f, physx::PxHalfPi/* PhysX's capsule is horizontal */));
	}

}
