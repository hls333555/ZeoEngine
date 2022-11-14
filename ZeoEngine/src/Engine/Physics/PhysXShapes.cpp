#include "ZEpch.h"
#include "Engine/Physics/PhysXShapes.h"

#include <extensions/PxRigidActorExt.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysicsMaterial.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/PhysXActor.h"
#include "Engine/Physics/PhysXUtils.h"

namespace ZeoEngine {

	void PhysXColliderShapeBase::SetPhysicsMaterial(AssetHandle physicsMaterialAsset)
	{
		auto physicsMaterial = AssetLibrary::LoadAsset<PhysicsMaterial>(physicsMaterialAsset);
		if (!physicsMaterial)
		{
			physicsMaterial = CreateRef<PhysicsMaterial>();
		}
		m_PhysicsMaterial = PhysXEngine::GetPhysics().createMaterial(physicsMaterial->GetStaticFriction(), physicsMaterial->GetDynamicFriction(), physicsMaterial->GetBounciness());
	}

	PhysXBoxColliderShape::PhysXBoxColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& boxComp = entity.GetComponent<BoxColliderComponent>();
		SetPhysicsMaterial(boxComp.PhysicsMaterialAsset);

		Vec3 scaledSize = entity.GetScale() * boxComp.Size;
		auto geometry = physx::PxBoxGeometry(scaledSize.x / 2.0f, scaledSize.y / 2.0f, scaledSize.z / 2.0f);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, GetPhysicsMaterial());
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !boxComp.bIsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxComp.bIsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(boxComp.Offset, Vec3(0.0f)));
	}

	void PhysXBoxColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

	PhysXSphereColliderShape::PhysXSphereColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& sphereComp = entity.GetComponent<SphereColliderComponent>();
		SetPhysicsMaterial(sphereComp.PhysicsMaterialAsset);

		const auto& scale = entity.GetScale();
		float largestScale = glm::max(scale.x, glm::max(scale.y, scale.z));
		auto geometry = physx::PxSphereGeometry(largestScale * sphereComp.Radius);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, GetPhysicsMaterial());
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !sphereComp.bIsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, sphereComp.bIsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(sphereComp.Offset, Vec3(0.0f)));
	}

	void PhysXSphereColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

	PhysXCapsuleColliderShape::PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor)
	{
		const auto& capsuleComp = entity.GetComponent<CapsuleColliderComponent>();
		SetPhysicsMaterial(capsuleComp.PhysicsMaterialAsset);

		const auto& scale = entity.GetScale();
		float radiusScale = glm::max(scale.x, scale.z);
		auto geometry = physx::PxCapsuleGeometry(capsuleComp.Radius * radiusScale, (capsuleComp.Height / 2.0f) * scale.y);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetRigidActor(), geometry, GetPhysicsMaterial());
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !capsuleComp.bIsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleComp.bIsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(capsuleComp.Offset, Vec3(0.0f, 0.0f, physx::PxHalfPi))); // PhysX's capsule is horizontal
	}

	void PhysXCapsuleColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

}
