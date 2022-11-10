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

}
