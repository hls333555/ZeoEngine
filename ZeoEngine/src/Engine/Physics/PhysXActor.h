#pragma once

#include "Engine/GameFramework/Entity.h"
#include "Engine/Physics/PhysicsTypes.h"

namespace physx
{
	class PxRigidActor;
}

namespace ZeoEngine {

	class PhysXColliderShapeBase;
	struct RigidBodyComponent;

	class PhysXActor
	{
		friend class PhysXScene;

	public:
		explicit PhysXActor(Entity entity);
		~PhysXActor();

		Entity GetEntity() const { return m_Entity; }

		physx::PxRigidActor& GetRigidActor() const { return *m_RigidActor; }

		void SetTranslation(const Vec3& translation, bool bAutoWake = true) const;
		void SetRotation(const Vec3& rotation, bool bAutoWake = true) const;
		void SetTransform(const Mat4& transform, bool bAutoWake = true) const;

		void SetKinematicTarget(const Vec3& targetPosition, const Vec3& targetRotation) const;

		bool IsDynamic() const;
		bool IsKinematic() const;
		void SetKinematic(bool bIsKinematic) const;

		bool IsSleeping() const;
		void WakeUp() const;
		void PutToSleep() const;

		bool IsGravityEnabled() const;
		void SetGravityEnabled(bool bEnable) const;
		bool IsSimulationEnabled() const;
		void SetSimulationEnabled(bool bEnable) const;

		float GetMass() const;
		float GetInverseMass() const;
		void SetMass(float mass) const;

		float GetLinearDamping() const;
		void SetLinearDamping(float damping) const;
		float GetAngularDamping() const;
		void SetAngularDamping(float damping) const;

		Vec3 GetLinearVelocity() const;
		void SetLinearVelocity(const Vec3& velocity) const;
		Vec3 GetAngularVelocity() const;
		void SetAngularVelocity(const Vec3& velocity) const;
		float GetMaxLinearVelocity() const;
		void SetMaxLinearVelocity(float maxVelocity) const;
		float GetMaxAngularVelocity() const;
		void SetMaxAngularVelocity(float maxVelocity) const;

		bool IsLockFlagSet(ActorLockFlag flag) const { return static_cast<U8>(flag) & m_LockFlags; }
		void SetLockFlag(ActorLockFlag flag, bool bValue, bool bForceWake = false);

		void AddForce(const Vec3& force, ForceMode forceMode) const;
		void AddTorque(const glm::vec3& torque, ForceMode forceMode) const;

	private:
		void CreateRigidActor();
		void AddCollider(ColliderType type);
		void SynchronizeTransform();

	private:
		Entity m_Entity;

		physx::PxRigidActor* m_RigidActor = nullptr;
		std::vector<Scope<PhysXColliderShapeBase>> m_Colliders;

		U8 m_LockFlags = 0;
	};

}
