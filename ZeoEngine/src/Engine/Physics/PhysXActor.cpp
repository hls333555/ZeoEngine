#include "ZEpch.h"
#include "Engine/Physics/PhysXActor.h"

#include <PxRigidStatic.h>
#include <PxRigidDynamic.h>
#include <PxPhysics.h>
#include <extensions/PxRigidBodyExt.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Physics/PhysXUtils.h"
#include "Engine/Physics/PhysXShapes.h"

namespace ZeoEngine {

	PhysXActor::PhysXActor(Entity entity)
		: m_Entity(entity)
	{
		CreateRigidActor();
	}

	PhysXActor::~PhysXActor()
	{
		for (const auto& [entityID, colliders] : m_Colliders)
		{
			for (const auto& collider : colliders)
			{
				collider->DetachFromActor(m_RigidActor);
			}
		}
		m_Colliders.clear();

		m_RigidActor->release();
		m_RigidActor = nullptr;
	}

	Vec3 PhysXActor::GetTranslation() const
	{
		const auto actorPose = m_RigidActor->getGlobalPose();
		return PhysXUtils::FromPhysXVector(actorPose.p);
	}

	Vec3 PhysXActor::GetRotation() const
	{
		const auto actorPose = m_RigidActor->getGlobalPose();
		return glm::eulerAngles(PhysXUtils::FromPhysXQuat(actorPose.q));
	}

	void PhysXActor::GetTransform(Vec3& outTranslation, Vec3& outRotation) const
	{
		const auto actorPose = m_RigidActor->getGlobalPose();
		outTranslation = PhysXUtils::FromPhysXVector(actorPose.p);
		outRotation = glm::eulerAngles(PhysXUtils::FromPhysXQuat(actorPose.q));
	}

	void PhysXActor::SetTranslation(const Vec3& translation, bool bAutoWake) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to move a non-dynamic physics actor.");
			return;
		}

		auto transform = m_RigidActor->getGlobalPose();
		transform.p = PhysXUtils::ToPhysXVector(translation);
		m_RigidActor->setGlobalPose(transform, bAutoWake);
	}

	void PhysXActor::SetRotation(const Vec3& rotation, bool bAutoWake) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to move a non-dynamic physics actor.");
			return;
		}

		auto transform = m_RigidActor->getGlobalPose();
		transform.q = PhysXUtils::ToPhysXQuat(Quat(rotation));
		m_RigidActor->setGlobalPose(transform, bAutoWake);
	}

	void PhysXActor::SetTransform(const Vec3& translation, const Vec3& rotation, bool bAutoWake) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to move a non-dynamic physics actor.");
			return;
		}

		const physx::PxTransform transform(PhysXUtils::ToPhysXVector(translation), PhysXUtils::ToPhysXQuat(Quat(rotation)));
		m_RigidActor->setGlobalPose(transform, bAutoWake);
	}

	void PhysXActor::SetKinematicTarget(const Vec3& targetPosition, const Vec3& targetRotation) const
	{
		if (!IsKinematic())
		{
			ZE_CORE_WARN("Trying to set kinematic target for a non-kinematic actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setKinematicTarget(PhysXUtils::ToPhysXTransform(targetPosition, targetRotation));
	}

	bool PhysXActor::IsDynamic() const
	{
		return m_RigidActor->is<physx::PxRigidDynamic>();
	}

	bool PhysXActor::IsKinematic() const
	{
		return IsDynamic() && m_RigidActor->is<physx::PxRigidDynamic>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
	}

	void PhysXActor::SetKinematic(bool bIsKinematic) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set kinematic of non-dynamic physics actor.");
			return;
		}

		m_RigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, bIsKinematic);
	}

	bool PhysXActor::IsSleeping() const
	{
		return IsDynamic() ? m_RigidActor->is<physx::PxRigidDynamic>()->isSleeping() : false;
	}

	void PhysXActor::WakeUp() const
	{
		if (IsDynamic())
		{
			m_RigidActor->is<physx::PxRigidDynamic>()->wakeUp();
		}
	}

	void PhysXActor::PutToSleep() const
	{
		if (IsDynamic())
		{
			m_RigidActor->is<physx::PxRigidDynamic>()->putToSleep();
		}
	}

	bool PhysXActor::IsGravityEnabled() const
	{
		return !m_RigidActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
	}

	void PhysXActor::SetGravityEnabled(bool bEnable) const
	{
		m_RigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !bEnable);
	}

	bool PhysXActor::IsSimulationEnabled() const
	{
		return !m_RigidActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION);
	}

	void PhysXActor::SetSimulationEnabled(bool bEnable) const
	{
		m_RigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, !bEnable);
	}

	float PhysXActor::GetMass() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get mass of non-dynamic physics actor.");
			return 0.0f;
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return rigidDynamic->getMass();
	}

	float PhysXActor::GetInverseMass() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get inverse mass of non-dynamic physics actor.");
			return 0.0f;
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return rigidDynamic->getInvMass();
	}

	void PhysXActor::SetMass(float mass) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set mass of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		physx::PxRigidBodyExt::setMassAndUpdateInertia(*rigidDynamic, mass);
	}

	float PhysXActor::GetLinearDamping() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get linear damping of non-dynamic physics actor.");
			return 0.0f;
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return rigidDynamic->getLinearDamping();
	}

	void PhysXActor::SetLinearDamping(float damping) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set linear damping of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setLinearDamping(damping);
	}

	float PhysXActor::GetAngularDamping() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get angular damping of non-dynamic physics actor.");
			return 0.0f;
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return rigidDynamic->getAngularDamping();
	}

	void PhysXActor::SetAngularDamping(float damping) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set angular damping of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setAngularDamping(damping);
	}

	Vec3 PhysXActor::GetLinearVelocity() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get linear velocity of non-dynamic physics actor.");
			return Vec3{ 0.0f };
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return PhysXUtils::FromPhysXVector(rigidDynamic->getLinearVelocity());
	}

	void PhysXActor::SetLinearVelocity(const Vec3& velocity) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set linear velocity of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setLinearVelocity(PhysXUtils::ToPhysXVector(velocity));
	}

	Vec3 PhysXActor::GetAngularVelocity() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get angular velocity of non-dynamic physics actor.");
			return Vec3{ 0.0f };
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return PhysXUtils::FromPhysXVector(rigidDynamic->getAngularVelocity());
	}

	void PhysXActor::SetAngularVelocity(const Vec3& velocity) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set angular velocity of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setAngularVelocity(PhysXUtils::ToPhysXVector(velocity));
	}

	float PhysXActor::GetMaxLinearVelocity() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get max linear velocity of non-dynamic physics actor.");
			return 0.0f;
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return rigidDynamic->getMaxLinearVelocity();
	}

	void PhysXActor::SetMaxLinearVelocity(float maxVelocity) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set max linear velocity of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setMaxLinearVelocity(maxVelocity);
	}

	float PhysXActor::GetMaxAngularVelocity() const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to get max angular velocity of non-dynamic physics actor.");
			return 0.0f;
		}

		const auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		return rigidDynamic->getMaxAngularVelocity();
	}

	void PhysXActor::SetMaxAngularVelocity(float maxVelocity) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to set max angular velocity of non-dynamic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setMaxAngularVelocity(maxVelocity);
	}

	void PhysXActor::SetLockFlag(ActorLockFlag flag, bool bValue, bool bForceWake)
	{
		if (!IsDynamic()) return;

		if (bValue)
		{
			m_LockFlags |= static_cast<U8>(flag);
		}
		else
		{
			m_LockFlags &= ~static_cast<U8>(flag);
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->setRigidDynamicLockFlags(static_cast<physx::PxRigidDynamicLockFlags>(m_LockFlags));
	}

	void PhysXActor::AddForce(const Vec3& force, ForceMode forceMode) const
	{
		if (!IsDynamic() || IsKinematic())
		{
			ZE_CORE_WARN("Trying to add force to a non-dynamic or kinematic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->addForce(PhysXUtils::ToPhysXVector(force), static_cast<physx::PxForceMode::Enum>(forceMode));
	}

	void PhysXActor::AddTorque(const glm::vec3& torque, ForceMode forceMode) const
	{
		if (!IsDynamic() || IsKinematic())
		{
			ZE_CORE_WARN("Trying to add torque to a non-dynamic or kinematic physics actor.");
			return;
		}

		auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
		rigidDynamic->addTorque(PhysXUtils::ToPhysXVector(torque), static_cast<physx::PxForceMode::Enum>(forceMode));
	}

	const std::vector<Scope<PhysXColliderShapeBase>>* PhysXActor::GetCollidersByEntity(Entity entity) const
	{
		const auto it = m_Colliders.find(entity.GetUUID());
		if (it == m_Colliders.end()) return nullptr;

		return &it->second;
	}

	void PhysXActor::CreateRigidActor()
	{
		auto& physics = PhysXEngine::GetPhysics();
		const auto& rigidBodyComp = m_Entity.GetComponent<RigidBodyComponent>();
		const auto& transform = PhysXUtils::ToPhysXTransform(m_Entity.GetWorldTransform());
		bool bIsStatic = rigidBodyComp.Type == RigidBodyComponent::BodyType::Static;
		if (bIsStatic)
		{
			m_RigidActor = physics.createRigidStatic(transform);
		}
		else
		{
			m_RigidActor = physics.createRigidDynamic(transform);
			m_RigidActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, rigidBodyComp.bReceiveSleepEvents);
			m_RigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, rigidBodyComp.bDisableSimulation);

			SetKinematic(rigidBodyComp.bIsKinematic);
			SetGravityEnabled(rigidBodyComp.bEnableGravity);
			SetLinearDamping(rigidBodyComp.LinearDamping);
			SetAngularDamping(rigidBodyComp.AngularDamping);

			SetLockFlag(ActorLockFlag::TranslationX, rigidBodyComp.bLockPositionX);
			SetLockFlag(ActorLockFlag::TranslationY, rigidBodyComp.bLockPositionY);
			SetLockFlag(ActorLockFlag::TranslationZ, rigidBodyComp.bLockPositionZ);
			SetLockFlag(ActorLockFlag::RotationX, rigidBodyComp.bLockRotationX);
			SetLockFlag(ActorLockFlag::RotationY, rigidBodyComp.bLockRotationY);
			SetLockFlag(ActorLockFlag::RotationZ, rigidBodyComp.bLockRotationZ);

			auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, rigidBodyComp.CollisionDetection == RigidBodyComponent::CollisionDetectionType::Continuous);
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, rigidBodyComp.CollisionDetection == RigidBodyComponent::CollisionDetectionType::ContinuousSpeculative);
			const auto& settings = PhysicsEngine::GetSettings();
			rigidDynamic->setSolverIterationCounts(settings.SolverPositionIterations, settings.SolverVelocityIterations);
		}

		Vec3 translation, rotation, scale;
		m_Entity.GetWorldTransform(translation, rotation, scale);
		const Mat4 scaleTransform = glm::scale(Mat4(1.0f), scale);
		AddColliders(m_Entity, static_cast<U32>(rigidBodyComp.CollisionDetection), scaleTransform);
		AddChildColliders(m_Entity, static_cast<U32>(rigidBodyComp.CollisionDetection), scaleTransform);

		if (!bIsStatic)
		{
			// Mass related properties must be updated after all colliders being added
			SetMass(rigidBodyComp.Mass);
		}

		m_RigidActor->userData = this;

#ifdef ZE_DEBUG
		const auto& coreComp = m_Entity.GetComponent<CoreComponent>();
		m_RigidActor->setName(coreComp.Name.c_str());
#endif
	}

	void PhysXActor::AddChildColliders(Entity entity, U32 collisionDetectionType, const Mat4& localTransform)
	{
		for (const UUID childID : entity.GetChildren())
		{
			Entity child = entity.GetScene().GetEntityByUUID(childID);
			if (child.HasComponent<RigidBodyComponent>()) continue;

			const Mat4 childLocalTransform = localTransform * child.GetTransform();
			AddColliders(child, collisionDetectionType, childLocalTransform);
			child.AddTagComponent<ChildColliderComponent>(); // This tag is added at runtime, so it will get cleared on end play
			AddChildColliders(child, collisionDetectionType, childLocalTransform);
		}
	}

	void PhysXActor::AddColliders(Entity entity, U32 collisionDetectionType, const Mat4& localTransform)
	{
		const UUID entityID = entity.GetUUID();
		if (entity.HasComponent<BoxColliderComponent>())
		{
			m_Colliders[entityID].emplace_back(CreateScope<PhysXBoxColliderShape>(entity, *this, collisionDetectionType, localTransform));
		}
		if (entity.HasComponent<SphereColliderComponent>())
		{
			m_Colliders[entityID].emplace_back(CreateScope<PhysXSphereColliderShape>(entity, *this, collisionDetectionType, localTransform));
		}
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			m_Colliders[entityID].emplace_back(CreateScope<PhysXCapsuleColliderShape>(entity, *this, collisionDetectionType, localTransform));
		}
	}

	void PhysXActor::SynchronizeTransform()
	{
		Vec3 translation, rotation;
		GetTransform(translation, rotation);
		// Physics actor's transform does not have scale property
		m_Entity.SetWorldTransform(translation, rotation);
		// We do not need to set transform back to physics actor through observer as synchronization is already done here
		// See m_PhysicsActorObserver in LevelObserverSystem::OnUpdate
		m_Entity.AddTagComponent<IgnoreSyncTransformComponent>();
	}

}
