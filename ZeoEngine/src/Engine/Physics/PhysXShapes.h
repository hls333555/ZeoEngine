#pragma once

#include <PxShape.h>

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class PhysXActor;

	class PhysXColliderShapeBase
	{
	public:
		virtual ~PhysXColliderShapeBase() = default;

		virtual void SetCollisionFilterData(const physx::PxFilterData& filterData) = 0;
		virtual void DetachFromActor(physx::PxRigidActor* actor) = 0;
		void Release() {}

	protected:
		const physx::PxMaterial& GetPhysicsMaterial(AssetHandle physicsMaterialAsset) const;
	};

	class PhysXBoxColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXBoxColliderShape(Entity entity, const PhysXActor& actor);

		virtual void SetCollisionFilterData(const physx::PxFilterData& filterData) override;
		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* m_Shape = nullptr;
	};

	class PhysXSphereColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXSphereColliderShape(Entity entity, const PhysXActor& actor);

		virtual void SetCollisionFilterData(const physx::PxFilterData& filterData) override;
		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* m_Shape = nullptr;
	};

	class PhysXCapsuleColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor);

		virtual void SetCollisionFilterData(const physx::PxFilterData& filterData) override;
		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* m_Shape = nullptr;
	};

}
