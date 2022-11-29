#pragma once

#include "Engine/GameFramework/Entity.h"

namespace physx
{
	class PxGeometry;
	class PxShape;
	class PxMaterial;
	class PxRigidActor;
	struct PxFilterData;
}

namespace ZeoEngine {

	class PhysXActor;
	struct ColliderComponentBase;
	struct RigidBodyComponent;

	class PhysXColliderShapeBase
	{
	public:
		virtual ~PhysXColliderShapeBase() = default;

		bool IsSimulationEnabled() const;
		void SetSimulationEnabled(bool bEnable) const;
		bool IsQueryEnabled() const;
		void SetQueryEnabled(bool bEnable) const;
		void DetachFromActor(physx::PxRigidActor* actor) const;

	protected:
		void CreateShape(const PhysXActor& actor, const physx::PxGeometry& geometry, const ColliderComponentBase& colliderComp, const RigidBodyComponent& rigidBodyComp, const Vec3& translation, const Vec3& rotation = Vec3{ 0.0f });

	private:
		physx::PxShape* m_Shape = nullptr;
	};

	class PhysXBoxColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXBoxColliderShape(Entity entity, const PhysXActor& actor);
	};

	class PhysXSphereColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXSphereColliderShape(Entity entity, const PhysXActor& actor);
	};

	class PhysXCapsuleColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor);
	};

}
