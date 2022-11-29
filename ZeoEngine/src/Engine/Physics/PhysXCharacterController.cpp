#include "ZEpch.h"
#include "Engine/Physics/PhysXCharacterController.h"

#include <characterkinematic/PxCapsuleController.h>
#include <characterkinematic/PxControllerManager.h>
#include <PxRigidDynamic.h>

#include "Engine/Physics/CollisionLayer.h"
#include "Engine/Physics/PhysXActor.h"
#include "Engine/Physics/PhysXUtils.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	namespace Utils	{

		static physx::PxShape* GetColliderShape(const physx::PxController& controller)
		{
			const auto* actor = controller.getActor();
			ZE_CORE_ASSERT(actor && actor->getNbShapes() == 1);

			physx::PxShape* shape;
			actor->getShapes(&shape, 1);
			return shape;
		}
		
	}

	class PhysXControllerFilterCallback : public physx::PxControllerFilterCallback
	{
	public:
		bool filter(const physx::PxController& a, const physx::PxController& b) override;
	};

	bool PhysXControllerFilterCallback::filter(const physx::PxController& a, const physx::PxController& b)
	{
		const auto filterDataA = Utils::GetColliderShape(a)->getSimulationFilterData();
		const auto filterDataB = Utils::GetColliderShape(b)->getSimulationFilterData();
		if (filterDataA.word0 & filterDataB.word1 || filterDataB.word0 & filterDataA.word1)
		{
			return true;
		}
		return false;
	}

	class PhysXControllerHitReport : public physx::PxUserControllerHitReport
	{
	public:
		void onShapeHit(const physx::PxControllerShapeHit& hit) override;
		void onControllerHit(const physx::PxControllersHit& hit) override;
		void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;
	};

	// This will not be called when a moving shape hits a non-moving CCT
	void PhysXControllerHitReport::onShapeHit(const physx::PxControllerShapeHit& hit)
	{
		const auto* controller = static_cast<PhysXCharacterController*>(hit.controller->getUserData());
		const auto* otherActor = static_cast<PhysXActor*>(hit.actor->userData);
		auto* otherCollider = static_cast<PhysXColliderShapeBase*>(hit.shape->userData);
		const CharacterControllerHit hitInfo{ otherActor->GetEntity().GetUUID(), otherCollider, nullptr, PhysXUtils::FromPhysXExtendedVector(hit.worldPos), PhysXUtils::FromPhysXVector(hit.worldNormal), PhysXUtils::FromPhysXVector(hit.dir), hit.length };
		ScriptEngine::OnCharacterControllerCollide(controller->GetEntity(), hitInfo);
	}

	void PhysXControllerHitReport::onControllerHit(const physx::PxControllersHit& hit)
	{
		const auto* controller = static_cast<PhysXCharacterController*>(hit.controller->getUserData());
		auto* otherController = static_cast<PhysXCharacterController*>(hit.other->getUserData());
		const CharacterControllerHit collisionInfo{ otherController->GetEntity().GetUUID(), nullptr, otherController, PhysXUtils::FromPhysXExtendedVector(hit.worldPos), PhysXUtils::FromPhysXVector(hit.worldNormal), PhysXUtils::FromPhysXVector(hit.dir), hit.length };
		ScriptEngine::OnCharacterControllerCollide(controller->GetEntity(), collisionInfo);
	}

	void PhysXControllerHitReport::onObstacleHit(const physx::PxControllerObstacleHit& hit)
	{
	}

	static PhysXControllerFilterCallback s_ControllerFilterCallback;
	static PhysXControllerHitReport s_ControllerHitReport;

	PhysXCharacterController::PhysXCharacterController(Entity entity, physx::PxControllerManager* controllerManager)
		: m_Entity(entity)
	{
		const auto& transformComp = entity.GetComponent<TransformComponent>();
		const auto& controllerComp = entity.GetComponent<CharacterControllerComponent>();
		const float radiusScale = glm::max(transformComp.Scale.x, transformComp.Scale.z);
		auto* physicsMaterial = PhysXUtils::ToPhysXMaterial(controllerComp.PhysicsMaterialAsset);

		physx::PxCapsuleControllerDesc desc;
		desc.position = PhysXUtils::ToPhysXExtendedVector(transformComp.Translation + controllerComp.Offset);
		desc.height = controllerComp.Height * transformComp.Scale.y;
		desc.radius = controllerComp.Radius * radiusScale;
		desc.slopeLimit = std::max(0.0f, cos(glm::radians(controllerComp.SlopeLimitAngle)));
		desc.contactOffset = controllerComp.SkinThickness;
		desc.stepOffset = controllerComp.StepOffset;
		desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
		desc.material = physicsMaterial;
		desc.reportCallback = &s_ControllerHitReport;
		ZE_CORE_ASSERT(desc.isValid());

		m_Controller = controllerManager->createController(desc);
		m_Controller->setUserData(this);
		SetFilterData(controllerComp);
	}

	PhysXCharacterController::~PhysXCharacterController()
	{
		m_Controller->release();
		m_Controller = nullptr;
	}

	Vec3 PhysXCharacterController::GetTranslation() const
	{
		return PhysXUtils::FromPhysXExtendedVector(m_Controller->getPosition());
	}

	void PhysXCharacterController::SetTranslation(const Vec3& translation) const
	{
		m_Controller->setPosition(PhysXUtils::ToPhysXExtendedVector(translation));
	}

	Vec3 PhysXCharacterController::GetFootPosition() const
	{
		return PhysXUtils::FromPhysXExtendedVector(m_Controller->getFootPosition());
	}

	void PhysXCharacterController::SetFootPosition(const Vec3& position) const
	{
		m_Controller->setFootPosition(PhysXUtils::ToPhysXExtendedVector(position));
	}

	Vec3 PhysXCharacterController::GetUpVector() const
	{
		return PhysXUtils::FromPhysXVector(m_Controller->getUpDirection());
	}

	void PhysXCharacterController::SetUpVector(const Vec3& up) const
	{
		m_Controller->setUpDirection(PhysXUtils::ToPhysXVector(up));
	}

	void PhysXCharacterController::Move(const Vec3& displacement)
	{
		m_Displacement += displacement;
	}

	void PhysXCharacterController::OnUpdate(DeltaTime dt)
	{
		m_CollisionFlags = m_Controller->move(PhysXUtils::ToPhysXVector(m_Displacement), m_MinMoveDistance, dt, m_ControllerFilters);

		m_Displacement = Vec3{ 0.0f };
	}

	void PhysXCharacterController::SetFilterData(const CharacterControllerComponent& controllerComp)
	{
		const U32 collisionLayerBit = ZE_BIT(controllerComp.CollisionLayer);
		const U32 collisionGroupBits = CollisionLayerManager::GetGroupLayerMask(controllerComp.CollidesWithGroup);

		physx::PxFilterData simulationFilterData; // For other character controllers
		simulationFilterData.word0 = collisionLayerBit;
		simulationFilterData.word1 = collisionGroupBits;
		physx::PxFilterData queryFilterData; // For other static/dynamic colliders as character controller uses overlap query against them
		queryFilterData.word0 = collisionLayerBit;

		auto* shape = Utils::GetColliderShape(*m_Controller);
		shape->setSimulationFilterData(simulationFilterData);
		shape->setQueryFilterData(queryFilterData);

		m_FilterData.word0 = collisionGroupBits;
		m_ControllerFilters.mFilterData = &m_FilterData;
		// TODO: Filter static/dynamic
		//QueryFlag flag = QueryFlag::Both;
		//m_ControllerFilters.mFilterFlags = physx::PxQueryFlags(static_cast<U16>(flag));
		m_ControllerFilters.mCCTFilterCallback = &s_ControllerFilterCallback;
	}

	void PhysXCharacterController::SynchronizeTransform()
	{
		const auto& controllerComp = m_Entity.GetComponent<CharacterControllerComponent>();
		const Vec3 translation = PhysXUtils::FromPhysXExtendedVector(m_Controller->getPosition()) - controllerComp.Offset;
		auto& transformComp = m_Entity.GetComponent<TransformComponent>();
		transformComp.Translation = translation;
		// Queue an update for bounds calculation as we do not call patch on TransformComponent
		// In fact, we do not need to set transform back to physics actor as synchronization is just done
		// See PhysicsObserver in LevelObserverSystem
		m_Entity.PatchComponent<BoundsComponent>();
	}

}
