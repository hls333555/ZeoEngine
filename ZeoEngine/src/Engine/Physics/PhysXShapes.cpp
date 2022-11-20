#include "ZEpch.h"
#include "Engine/Physics/PhysXShapes.h"

#include <extensions/PxRigidActorExt.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysicsMaterial.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/PhysXActor.h"
#include "Engine/Physics/PhysXUtils.h"

namespace ZeoEngine {

	const physx::PxMaterial& PhysXColliderShapeBase::GetPhysicsMaterial(AssetHandle physicsMaterialAsset) const
	{
		const auto physicsMaterial = AssetLibrary::LoadAsset<PhysicsMaterial>(physicsMaterialAsset);
		if (physicsMaterial)
		{
			return *physicsMaterial->m_PhysicsMaterial;
		}
		else
		{
			const auto defaultPhysicsMaterial = PhysXEngine::GetDefaultPhysicsMaterial();
			ZE_CORE_ASSERT(defaultPhysicsMaterial);
			return *defaultPhysicsMaterial->m_PhysicsMaterial;
		}
	}

	PhysXBoxColliderShape::PhysXBoxColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& boxComp = entity.GetComponent<BoxColliderComponent>();

		Vec3 scaledSize = entity.GetScale() * boxComp.Size;
		auto geometry = physx::PxBoxGeometry(scaledSize.x / 2.0f, scaledSize.y / 2.0f, scaledSize.z / 2.0f);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, GetPhysicsMaterial(boxComp.PhysicsMaterialAsset));
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !boxComp.bIsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxComp.bIsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(boxComp.Offset, Vec3(0.0f)));
		m_Shape->userData = this;
	}

	void PhysXBoxColliderShape::SetCollisionFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void PhysXBoxColliderShape::SetQueryFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setQueryFilterData(filterData);
	}

	void PhysXBoxColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

	PhysXSphereColliderShape::PhysXSphereColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& sphereComp = entity.GetComponent<SphereColliderComponent>();

		const auto& scale = entity.GetScale();
		float largestScale = glm::max(scale.x, glm::max(scale.y, scale.z));
		auto geometry = physx::PxSphereGeometry(sphereComp.Radius * largestScale);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, GetPhysicsMaterial(sphereComp.PhysicsMaterialAsset));
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !sphereComp.bIsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, sphereComp.bIsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(sphereComp.Offset, Vec3(0.0f)));
		m_Shape->userData = this;
	}

	void PhysXSphereColliderShape::SetCollisionFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
		m_Shape->setQueryFilterData(filterData);
	}

	void PhysXSphereColliderShape::SetQueryFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setQueryFilterData(filterData);
	}

	void PhysXSphereColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

	PhysXCapsuleColliderShape::PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& capsuleComp = entity.GetComponent<CapsuleColliderComponent>();

		const auto& scale = entity.GetScale();
		float radiusScale = glm::max(scale.x, scale.z);
		auto geometry = physx::PxCapsuleGeometry(capsuleComp.Radius * radiusScale, (capsuleComp.Height / 2.0f) * scale.y);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, GetPhysicsMaterial(capsuleComp.PhysicsMaterialAsset));
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !capsuleComp.bIsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleComp.bIsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(capsuleComp.Offset, Vec3(0.0f, 0.0f, physx::PxHalfPi))); // PhysX's capsule is horizontal
		m_Shape->userData = this;
	}

	void PhysXCapsuleColliderShape::SetCollisionFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
		m_Shape->setQueryFilterData(filterData);
	}

	void PhysXCapsuleColliderShape::SetQueryFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setQueryFilterData(filterData);
	}

	void PhysXCapsuleColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

}
