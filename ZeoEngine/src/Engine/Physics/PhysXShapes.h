#pragma once

#include <PxShape.h>
#include <PxMaterial.h>

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class PhysXActor;

	class PhysXColliderShapeBase
	{
	public:
		virtual ~PhysXColliderShapeBase() = default;

		physx::PxMaterial& GetPhysicsMaterial() const { return *m_PhysicsMaterial; }
		void SetPhysicsMaterial(AssetHandle physicsMaterialAsset);

		virtual void DetachFromActor(physx::PxRigidActor* actor) = 0;
		void Release() { m_PhysicsMaterial->release(); }

	private:
		physx::PxMaterial* m_PhysicsMaterial = nullptr;
	};

	class PhysXBoxColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXBoxColliderShape(Entity entity, const PhysXActor& actor);

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* m_Shape = nullptr;
	};

	class PhysXSphereColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXSphereColliderShape(Entity entity, const PhysXActor& actor);

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* m_Shape = nullptr;
	};

}
