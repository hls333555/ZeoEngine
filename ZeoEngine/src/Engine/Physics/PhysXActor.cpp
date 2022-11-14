#include "ZEpch.h"
#include "Engine/Physics/PhysXActor.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXEngine.h"
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
		for (const auto& collider : m_Colliders)
		{
			collider->DetachFromActor(m_RigidActor);
			collider->Release();
		}
		m_Colliders.clear();

		m_RigidActor->release();
		m_RigidActor = nullptr;
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

	void PhysXActor::SetTransform(const Mat4& transform, bool bAutoWake) const
	{
		if (!IsDynamic())
		{
			ZE_CORE_WARN("Trying to move a non-dynamic physics actor.");
			return;
		}

		m_RigidActor->setGlobalPose(PhysXUtils::ToPhysXTransform(transform), bAutoWake);
	}

	bool PhysXActor::IsDynamic() const
	{
		const auto& rigidBodyComp = m_Entity.GetComponent<RigidBodyComponent>();
		return rigidBodyComp.Type == RigidBodyComponent::BodyType::Dynamic;
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

	bool PhysXActor::IsGravityEnabled() const
	{
		return !m_RigidActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
	}

	void PhysXActor::SetGravityEnabled(bool bEnable) const
	{
		m_RigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !bEnable);
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

	void PhysXActor::AddCollider(ColliderType type)
	{
		switch (type)
		{
			case ColliderType::Box:
				m_Colliders.emplace_back(CreateScope<PhysXBoxColliderShape>(m_Entity, *this));
				break;
			case ColliderType::Sphere:
				m_Colliders.emplace_back(CreateScope<PhysXSphereColliderShape>(m_Entity, *this));
				break;
			case ColliderType::Capsule:
				m_Colliders.emplace_back(CreateScope<PhysXCapsuleColliderShape>(m_Entity, *this));
				break;
		}
	}

	void PhysXActor::CreateRigidActor()
	{
		auto& physics = PhysXEngine::GetPhysics();
		const auto& rigidBodyComp = m_Entity.GetComponent<RigidBodyComponent>();
		// TODO: Entity hierarchy
		const auto& transform = PhysXUtils::ToPhysXTransform(m_Entity.GetTransform());
		if (rigidBodyComp.Type == RigidBodyComponent::BodyType::Static)
		{
			m_RigidActor = physics.createRigidStatic(transform);
		}
		else
		{
			m_RigidActor = physics.createRigidDynamic(transform);

			SetKinematic(rigidBodyComp.bIsKinematic);
			SetMass(rigidBodyComp.Mass);
			SetGravityEnabled(rigidBodyComp.bEnableGravity);
			SetLinearDamping(rigidBodyComp.LinearDamping);
			SetAngularDamping(rigidBodyComp.AngularDamping);

			auto* rigidDynamic = m_RigidActor->is<physx::PxRigidDynamic>();
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, rigidBodyComp.CollisionDetection == RigidBodyComponent::CollisionDetectionType::Continuous);
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, rigidBodyComp.CollisionDetection == RigidBodyComponent::CollisionDetectionType::ContinuousSpeculative);
		}

		if (m_Entity.HasComponent<BoxColliderComponent>())
		{
			AddCollider(ColliderType::Box);
		}
		if (m_Entity.HasComponent<SphereColliderComponent>())
		{
			AddCollider(ColliderType::Sphere);
		}
		if (m_Entity.HasComponent<CapsuleColliderComponent>())
		{
			AddCollider(ColliderType::Capsule);
		}

		m_RigidActor->userData = this;

#ifdef ZE_DEBUG
		const auto& coreComp = m_Entity.GetComponent<CoreComponent>();
		m_RigidActor->setName(coreComp.Name.c_str());
#endif
	}

	void PhysXActor::SynchronizeTransform()
	{
		const auto actorPose = m_RigidActor->getGlobalPose();
		const Vec3 translation = PhysXUtils::FromPhysXVector(actorPose.p);
		const Vec3 rotation = glm::eulerAngles(PhysXUtils::FromPhysXQuat(actorPose.q));
		auto& transformComp = m_Entity.GetComponent<TransformComponent>();
		transformComp.Translation = translation;
		transformComp.Rotation = glm::degrees(rotation);
		// Queue an update for bounds calculation as we do not call patch on TransformComponent
		// In fact, we do not need to set transform back to physics actor as synchronization is just done
		// See PhysicsObserver in LevelObserverSystem
		m_Entity.PatchComponent<BoundsComponent>();
	}

}
