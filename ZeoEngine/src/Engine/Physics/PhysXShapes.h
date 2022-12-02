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

	class PhysXColliderShapeBase
	{
	public:
		virtual ~PhysXColliderShapeBase() = default;

		Entity GetEntity() const { return m_Entity; }

		bool IsSimulationEnabled() const;
		void SetSimulationEnabled(bool bEnable) const;
		bool IsQueryEnabled() const;
		void SetQueryEnabled(bool bEnable) const;
		void DetachFromActor(physx::PxRigidActor* actor) const;
		void SetTransform(const Mat4& transform) const;

	protected:
		void CreateShape(Entity entity, const PhysXActor& actor, const physx::PxGeometry& geometry, const ColliderComponentBase& colliderComp, U32 collisionDetectionType, const Mat4& transform);

		virtual Mat4 GetOffsetTransform(const Mat4& transform) const = 0;

	private:
		Entity m_Entity;

		physx::PxShape* m_Shape = nullptr;
	};

	class PhysXBoxColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXBoxColliderShape(Entity entity, const PhysXActor& actor, U32 collisionDetectionType, const Mat4& transform);

	private:
		virtual Mat4 GetOffsetTransform(const Mat4& transform) const override;
	};

	class PhysXSphereColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXSphereColliderShape(Entity entity, const PhysXActor& actor, U32 collisionDetectionType, const Mat4& transform);

	private:
		virtual Mat4 GetOffsetTransform(const Mat4& transform) const override;
	};

	class PhysXCapsuleColliderShape : public PhysXColliderShapeBase
	{
	public:
		PhysXCapsuleColliderShape(Entity entity, const PhysXActor& actor, U32 collisionDetectionType, const Mat4& transform);

	private:
		virtual Mat4 GetOffsetTransform(const Mat4& transform) const override;
	};

}
