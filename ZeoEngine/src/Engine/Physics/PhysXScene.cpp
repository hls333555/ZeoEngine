#include "ZEpch.h"
#include "Engine/Physics/PhysXScene.h"

#include <PxScene.h>
#include <characterkinematic/PxControllerManager.h>
#include <PxPhysics.h>
#include <PxRigidActor.h>
#include <PxMaterial.h>
#include <PxSimulationEventCallback.h>
#include <extensions/PxBroadPhaseExt.h>

#include "PhysXCharacterController.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/PhysXUtils.h"
#include "Engine/Physics/PhysXActor.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Utils/DebugDrawUtils.h"
#include "Engine/Utils/SceneUtils.h"

namespace ZeoEngine {

	class PhysXBroadPhaseCallback : public physx::PxBroadPhaseCallback
	{
	public:
		void onObjectOutOfBounds(physx::PxShape& shape, physx::PxActor& actor) override;
		void onObjectOutOfBounds(physx::PxAggregate& aggregate) override;
	};

	// https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/RigidBodyCollision.html#broad-phase-callback
	void PhysXBroadPhaseCallback::onObjectOutOfBounds(physx::PxShape& shape, physx::PxActor& actor)
	{
		if (const auto* rigidActor = actor.is<physx::PxRigidActor>())
		{
#ifdef ZE_DEBUG
			const char* name = rigidActor->getName();
#else
			const auto* physXActor = static_cast<PhysXActor*>(rigidActor->userData);
			const std::string& name = physXActor->GetEntity().GetName();
#endif
			ZE_CORE_WARN("Collision detection is disabled for physics actor \"{}\" due to being out of world bounds", name);
		}
	}

	void PhysXBroadPhaseCallback::onObjectOutOfBounds(physx::PxAggregate& aggregate)
	{
	}

	class PhysXSimulationEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
		virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
		virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
		virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
		virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
		virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
	};

	void PhysXSimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
	}

	void PhysXSimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
	}

	void PhysXSimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
	}

	void PhysXSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		if (!SceneUtils::IsLevelPlaying()) return;

		auto removedActor0 = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
		auto removedActor1 = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;
		if (removedActor0 || removedActor1) return;

		const auto* actor0 = static_cast<PhysXActor*>(pairHeader.actors[0]->userData);
		const auto* actor1 = static_cast<PhysXActor*>(pairHeader.actors[1]->userData);
		if (!actor0 || !actor1) return;

		bool bActor0ScriptClassValid = ScriptEngine::IsEntityScriptClassValid(actor0->GetEntity());
		bool bActor1ScriptClassValid = ScriptEngine::IsEntityScriptClassValid(actor1->GetEntity());
		if (!bActor0ScriptClassValid && !bActor1ScriptClassValid) return;

		for (U32 i = 0; i < nbPairs; ++i)
		{
			const auto& contactPair = pairs[i];

			auto removedShape0 = contactPair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0;
			auto removedShape1 = contactPair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1;
			if (removedShape0 || removedShape1) continue;

			const auto* shape0 = contactPair.shapes[0];
			auto* collider0 = static_cast<PhysXColliderShapeBase*>(shape0->userData);
			const auto* shape1 = contactPair.shapes[1];
			auto* collider1 = static_cast<PhysXColliderShapeBase*>(shape1->userData);

			physx::PxContactPairPoint contacts[PHYSICS_MAX_CONTACT_POINTS];
			const U32 numContacts = contactPair.extractContacts(contacts, PHYSICS_MAX_CONTACT_POINTS);
			CollisionInfo collisionInfo;
			collisionInfo.NumContacts = numContacts;
			for (U32 j = 0; j < numContacts; ++j)
			{
				const auto& contact = contacts[j];

				auto* physxMaterial0 = shape0->getMaterialFromInternalFaceIndex(contact.internalFaceIndex0);
				const auto physicsMaterialAsset0 = static_cast<PhysicsMaterial*>(physxMaterial0->userData)->GetHandle();
				auto* physxMaterial1 = shape1->getMaterialFromInternalFaceIndex(contact.internalFaceIndex1);
				const auto physicsMaterialAsset1 = static_cast<PhysicsMaterial*>(physxMaterial1->userData)->GetHandle();
				collisionInfo.Contacts[j] = { PhysXUtils::FromPhysXVector(contact.position), PhysXUtils::FromPhysXVector(contact.normal), contact.separation, { physicsMaterialAsset0, physicsMaterialAsset1 } };
			}
			if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if (bActor0ScriptClassValid)
				{
					collisionInfo.OtherEntity = actor1->GetEntity().GetUUID();
					collisionInfo.OtherCollider = collider1;
					ScriptEngine::OnCollisionBegin(actor0->GetEntity(), collisionInfo);
				}
				if (bActor1ScriptClassValid)
				{
					collisionInfo.OtherEntity = actor0->GetEntity().GetUUID();
					collisionInfo.OtherCollider = collider0;
					ScriptEngine::OnCollisionBegin(actor1->GetEntity(), collisionInfo);
				}
			}
			else if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (bActor0ScriptClassValid)
				{
					collisionInfo.OtherEntity = actor1->GetEntity().GetUUID();
					collisionInfo.OtherCollider = collider1;
					ScriptEngine::OnCollisionEnd(actor0->GetEntity(), collisionInfo);
				}
				if (bActor1ScriptClassValid)
				{
					collisionInfo.OtherEntity = actor0->GetEntity().GetUUID();
					collisionInfo.OtherCollider = collider0;
					ScriptEngine::OnCollisionEnd(actor1->GetEntity(), collisionInfo);
				}
			}
		}
	}

	void PhysXSimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		if (!SceneUtils::IsLevelPlaying()) return;

		for (U32 i = 0; i < count; ++i)
		{
			const auto& pair = pairs[i];

			auto removedTrigger = pair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER;
			auto removedOther = pair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER;
			if (removedTrigger || removedOther) continue;

			const auto* triggerActor = static_cast<PhysXActor*>(pair.triggerActor->userData);
			const auto* otherActor = static_cast<PhysXActor*>(pair.otherActor->userData);
			if (!triggerActor || !otherActor) continue;

			bool bTriggerActorScriptClassValid = ScriptEngine::IsEntityScriptClassValid(triggerActor->GetEntity());
			bool bOtherActorScriptClassValid = ScriptEngine::IsEntityScriptClassValid(otherActor->GetEntity());
			if (!bTriggerActorScriptClassValid && !bOtherActorScriptClassValid) continue;

			if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if (bTriggerActorScriptClassValid)
				{
					ScriptEngine::OnTriggerBegin(triggerActor->GetEntity(), otherActor->GetEntity());
				}
				if (bOtherActorScriptClassValid)
				{
					ScriptEngine::OnTriggerBegin(otherActor->GetEntity(), triggerActor->GetEntity());
				}
			}
			else if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (bTriggerActorScriptClassValid)
				{
					ScriptEngine::OnTriggerEnd(triggerActor->GetEntity(), otherActor->GetEntity());
				}
				if (bOtherActorScriptClassValid)
				{
					ScriptEngine::OnTriggerEnd(otherActor->GetEntity(), triggerActor->GetEntity());
				}
			}
		}
	}

	void PhysXSimulationEventCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
	}

	/**
	 * Whenever the scene query intersects a shape, filtering is performed in the following order:
	 * For non-batched queries only:
	 * First, fixed function filtering is performed, query's QueriesFor will be used to test against shape's layer ID, discarding non-passed shapes
	 * Second, PhysXQueryFilterCallback::preFilter will be invoked in which query's BlockingHitLayerMask will be used to test against shape's layer ID to determine whether current hit is blocking or not
	 * For further information, take a look at https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/SceneQueries.html
	 */
	class PhysXQueryFilterCallback : public physx::PxQueryFilterCallback
	{
	public:
		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;
	};

	physx::PxQueryHitType::Enum PhysXQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
	{
		const auto shapeFilterData = shape->getQueryFilterData();
		if (filterData.word1 & shapeFilterData.word0)
		{
			return physx::PxQueryHitType::eBLOCK;
		}

		return physx::PxQueryHitType::eTOUCH;
	}

	physx::PxQueryHitType::Enum PhysXQueryFilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
	{
		ZE_CORE_ASSERT(false);
		return physx::PxQueryHitType::eNONE; // Generally, this function is never called as we do not provide the physx::PxQueryFlag::ePOSTFILTER flag
	}

	static physx::PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
	{
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		if (filterData0.word2 == static_cast<U32>(RigidBodyComponent::CollisionDetectionType::Continuous) || filterData1.word2 == static_cast<U32>(RigidBodyComponent::CollisionDetectionType::Continuous))
		{
			pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		}

		// Collision will only happen if the pair's collision layers are in each other's collision group
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
			pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eSUPPRESS;
	}

	static PhysXBroadPhaseCallback s_BroadPhaseCallback;
	static PhysXSimulationEventCallback s_SimulationEventCallback;
	static PhysXQueryFilterCallback s_QueryFilterCallback;

	namespace Utils	{

		static void ExtractHit(const Scene* scene, RaycastHit& outHit, bool bIsBlockingHit, const physx::PxLocationHit& hit, bool bDrawDebug, float duration)
		{
			const auto* actor = static_cast<PhysXActor*>(hit.actor->userData);
			auto* collider = static_cast<PhysXColliderShapeBase*>(hit.shape->userData);
			outHit.bIsBlockingHit = bIsBlockingHit;
			outHit.HitEntity = actor->GetEntity().GetUUID();
			outHit.HitCollider = collider;
			outHit.Position = PhysXUtils::FromPhysXVector(hit.position);
			outHit.Normal = PhysXUtils::FromPhysXVector(hit.normal);
			outHit.Distance = hit.distance;
			if (bDrawDebug)
			{
				DebugDrawUtils::DrawPoint(*scene, outHit.Position, bIsBlockingHit ? Vec3(1.0f, 0.0f, 0.0f) : Vec3(0.0f, 1.0f, 0.0f), 20, duration);
			}
		}

		static void ExtractHit(OverlapHit& outHit, const physx::PxOverlapHit& hit)
		{
			const auto* actor = static_cast<PhysXActor*>(hit.actor->userData);
			auto* collider = static_cast<PhysXColliderShapeBase*>(hit.shape->userData);
			outHit.HitEntity = actor->GetEntity().GetUUID();
			outHit.HitCollider = collider;
		}
		
	}

	PhysXScene::PhysXScene(Scene* scene)
		: m_Scene(scene)
	{
		auto& physics = PhysXEngine::GetPhysics();
		const auto& settings = PhysicsEngine::GetSettings();
		m_SubStepTime = settings.FixedDeltaTime;

		physx::PxSceneDesc sceneDesc(physics.getTolerancesScale());
		sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
		sceneDesc.broadPhaseType = static_cast<physx::PxBroadPhaseType::Enum>(settings.BroadphaseAlgorithm);
		sceneDesc.broadPhaseCallback = &s_BroadPhaseCallback;
		sceneDesc.frictionType = static_cast<physx::PxFrictionType::Enum>(settings.FrictionModel);
		sceneDesc.solverType = static_cast<physx::PxSolverType::Enum>(settings.SolverModel);
		sceneDesc.filterShader = FilterShader;
		sceneDesc.simulationEventCallback = &s_SimulationEventCallback;
		sceneDesc.cpuDispatcher = PhysXEngine::GetCPUDispatcher();
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		if (settings.bEnableEnhancedDeterminism)
		{
			sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		}

		ZE_CORE_ASSERT(sceneDesc.isValid());

		m_PhysicsScene = physics.createScene(sceneDesc);
		if (auto* pvdClient = m_PhysicsScene->getScenePvdClient())
		{
			pvdClient->setScenePvdFlags(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES | physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS);
		}
		m_PhysicsControllerManager = PxCreateControllerManager(*m_PhysicsScene);
		// Turn off as stated in https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/CharacterControllers.html#overlap-recovery-module
		// When penetration happens, turn on this again
		m_PhysicsControllerManager->setPreciseSweeps(false);

		CreateRegions();
	}

	PhysXScene::~PhysXScene()
	{
		m_Actors.clear();
		m_CharacterControllers.clear();

		m_PhysicsControllerManager->release();
		m_PhysicsControllerManager = nullptr;
		m_PhysicsScene->release();
		m_PhysicsScene = nullptr;
	}

	void PhysXScene::Simulate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		// TODO: PhysXActor::OnFixedUpdate

		{
			ZE_PROFILE_FUNC("Update character controllers");

			// TODO: Test this, should be not depending on framerate
			// Compute overlap information to automatically separate overlapping characters in the next controller move call
			m_PhysicsControllerManager->computeInteractions(dt);
			for (const auto& controller : m_CharacterControllers)
			{
				controller->OnUpdate(dt);
			}
		}

		if (Advance(dt))
		{
			U32 numActiveActors;
			auto** activeActors = m_PhysicsScene->getActiveActors(numActiveActors);
			for (uint32_t i = 0; i < numActiveActors; ++i)
			{
				auto* actor = static_cast<PhysXActor*>(activeActors[i]->userData);
				if (actor && !actor->IsSleeping())
				{
					actor->SynchronizeTransform();
				}
			}

			for (const auto& controller : m_CharacterControllers)
			{
				controller->SynchronizeTransform();
			}
		}
	}

	PhysXActor* PhysXScene::GetActor(Entity entity) const
	{
		for (const auto& actor : m_Actors)
		{
			if (actor->GetEntity() == entity)
			{
				return actor.get();
			}
		}

		return nullptr;
	}

	PhysXActor* PhysXScene::CreateActor(Entity entity)
	{
		auto* foundActor = GetActor(entity);
		if (foundActor) return foundActor;

		auto actor = CreateScope<PhysXActor>(entity);
		auto* actorPtr = actor.get();
		// TODO: Entity hierarchy
		if (entity.HasComponent<RigidBodyComponent>())
		{
			
		}
		m_Actors.emplace_back(std::move(actor));
		m_PhysicsScene->addActor(actorPtr->GetRigidActor());
		return actorPtr;
	}

	void PhysXScene::DestroyActor(Entity entity)
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end(); ++it)
		{
			const auto& actor = *it;
			if (actor->GetEntity() == entity)
			{
				m_PhysicsScene->removeActor(actor->GetRigidActor());
				m_Actors.erase(it);
				return;
			}
		}
	}

	PhysXCharacterController* PhysXScene::GetCharacterController(Entity entity) const
	{
		for (const auto& controller : m_CharacterControllers)
		{
			if (controller->GetEntity() == entity)
			{
				return controller.get();
			}
		}

		return nullptr;
	}

	PhysXCharacterController* PhysXScene::CreateCharacterController(Entity entity)
	{
		auto* foundController = GetCharacterController(entity);
		if (foundController) return foundController;

		auto controller = CreateScope<PhysXCharacterController>(entity, m_PhysicsControllerManager);
		auto* controllerPtr = controller.get();
		m_CharacterControllers.emplace_back(std::move(controller));
		return controllerPtr;
	}

	void PhysXScene::DestroyCharacterController(Entity entity)
	{
		for (auto it = m_CharacterControllers.begin(); it != m_CharacterControllers.end(); ++it)
		{
			const auto& controller = *it;
			if (controller->GetEntity() == entity)
			{
				m_CharacterControllers.erase(it);
				return;
			}
		}
	}

	Vec3 PhysXScene::GetGravity() const
	{
		return PhysXUtils::FromPhysXVector(m_PhysicsScene->getGravity());
	}

	void PhysXScene::SetGravity(const Vec3& gravity) const
	{
		m_PhysicsScene->setGravity(PhysXUtils::ToPhysXVector(gravity));
	}

	bool PhysXScene::Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, RaycastHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxRaycastBuffer hitInfo;
		bool bResult = RaycastInternal(origin, direction, maxDistance, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(m_Scene, outHit, true, hitInfo.block, bDrawDebug, duration);
		}
		if (bDrawDebug)
		{
			const Vec3 target = origin + direction * maxDistance;
			DebugDrawUtils::DrawArrow(*m_Scene, origin, target, Vec3{ 1.0f }, 0.1f, duration);
		}

		return bResult;
	}

	bool PhysXScene::RaycastMulti(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, std::vector<RaycastHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxRaycastHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxRaycastBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = RaycastInternal(origin, direction, maxDistance, filter, hitInfo);
		if (bDrawDebug)
		{
			const Vec3 target = origin + direction * maxDistance;
			DebugDrawUtils::DrawArrow(*m_Scene, origin, target, Vec3{ 1.0f }, 0.1f, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		if (hitInfo.hasBlock)
		{
			RaycastHit hit;
			Utils::ExtractHit(m_Scene, hit, true, hitInfo.block, bDrawDebug, duration);
			outHits.emplace_back(hit);
		}
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			RaycastHit hit;
			Utils::ExtractHit(m_Scene, hit, false, hitInfo.touches[i], bDrawDebug, duration);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::BoxSweep(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxSweepBuffer hitInfo;
		bool bResult = BoxSweepInternal(center, extent, rotation, sweepDirection, sweepDistance, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(m_Scene, outHit, true, hitInfo.block, bDrawDebug, duration);
		}
		if (bDrawDebug)
		{
			const Vec3 target = center + sweepDirection * sweepDistance;
			DebugDrawUtils::DrawBox(*m_Scene, center, extent, Vec3{ 1.0f }, rotation, duration);
			DebugDrawUtils::DrawArrow(*m_Scene, center, target, Vec3{ 1.0f }, 0.1f, duration);
			DebugDrawUtils::DrawBox(*m_Scene, target, extent, Vec3{ 1.0f }, rotation, duration);
		}

		return bResult;
	}

	bool PhysXScene::BoxSweepMulti(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxSweepHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxSweepBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = BoxSweepInternal(center, extent, rotation, sweepDirection, sweepDistance, filter, hitInfo);
		if (bDrawDebug)
		{
			const Vec3 target = center + sweepDirection * sweepDistance;
			DebugDrawUtils::DrawBox(*m_Scene, center, extent, Vec3{ 1.0f }, rotation, duration);
			DebugDrawUtils::DrawArrow(*m_Scene, center, target, Vec3{ 1.0f }, 0.1f, duration);
			DebugDrawUtils::DrawBox(*m_Scene, target, extent, Vec3{ 1.0f }, rotation, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		if (hitInfo.hasBlock)
		{
			SweepHit hit;
			Utils::ExtractHit(m_Scene, hit, true, hitInfo.block, bDrawDebug, duration);
			outHits.emplace_back(hit);
		}
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			SweepHit hit;
			Utils::ExtractHit(m_Scene, hit, false, hitInfo.touches[i], bDrawDebug, duration);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::SphereSweep(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxSweepBuffer hitInfo;
		bool bResult = SphereSweepInternal(center, radius, sweepDirection, sweepDistance, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(m_Scene, outHit, true, hitInfo.block, bDrawDebug, duration);
		}
		if (bDrawDebug)
		{
			const Vec3 target = center + sweepDirection * sweepDistance;
			DebugDrawUtils::DrawSphere(*m_Scene, center, Vec3{ 1.0f }, radius, duration);
			DebugDrawUtils::DrawArrow(*m_Scene, center, target, Vec3{ 1.0f }, 0.1f, duration);
			DebugDrawUtils::DrawSphere(*m_Scene, target, Vec3{ 1.0f }, radius, duration);
		}

		return bResult;
	}

	bool PhysXScene::SphereSweepMulti(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxSweepHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxSweepBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = SphereSweepInternal(center, radius, sweepDirection, sweepDistance, filter, hitInfo);
		if (bDrawDebug)
		{
			const Vec3 target = center + sweepDirection * sweepDistance;
			DebugDrawUtils::DrawSphere(*m_Scene, center, Vec3{ 1.0f }, radius, duration);
			DebugDrawUtils::DrawArrow(*m_Scene, center, target, Vec3{ 1.0f }, 0.1f, duration);
			DebugDrawUtils::DrawSphere(*m_Scene, target, Vec3{ 1.0f }, radius, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		if (hitInfo.hasBlock)
		{
			SweepHit hit;
			Utils::ExtractHit(m_Scene, hit, true, hitInfo.block, bDrawDebug, duration);
			outHits.emplace_back(hit);
		}
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			SweepHit hit;
			Utils::ExtractHit(m_Scene, hit, false, hitInfo.touches[i], bDrawDebug, duration);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::CapsuleSweep(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxSweepBuffer hitInfo;
		bool bResult = CapsuleSweepInternal(center, radius, height, rotation, sweepDirection, sweepDistance, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(m_Scene, outHit, true, hitInfo.block, bDrawDebug, duration);
		}
		if (bDrawDebug)
		{
			const Vec3 target = center + sweepDirection * sweepDistance;
			DebugDrawUtils::DrawCapsule(*m_Scene, center, Vec3{ 1.0f }, radius, height, rotation, 32, duration);
			DebugDrawUtils::DrawArrow(*m_Scene, center, target, Vec3{ 1.0f }, 0.1f, duration);
			DebugDrawUtils::DrawCapsule(*m_Scene, target, Vec3{ 1.0f }, radius, height, rotation, 32, duration);
		}

		return bResult;
	}

	bool PhysXScene::CapsuleSweepMulti(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxSweepHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxSweepBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = CapsuleSweepInternal(center, radius, height, rotation, sweepDirection, sweepDistance, filter, hitInfo);
		if (bDrawDebug)
		{
			const Vec3 target = center + sweepDirection * sweepDistance;
			DebugDrawUtils::DrawCapsule(*m_Scene, center, Vec3{ 1.0f }, radius, height, rotation, 32, duration);
			DebugDrawUtils::DrawArrow(*m_Scene, center, target, Vec3{ 1.0f }, 0.1f, duration);
			DebugDrawUtils::DrawCapsule(*m_Scene, target, Vec3{ 1.0f }, radius, height, rotation, 32, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		if (hitInfo.hasBlock)
		{
			SweepHit hit;
			Utils::ExtractHit(m_Scene, hit, true, hitInfo.block, bDrawDebug, duration);
			outHits.emplace_back(hit);
		}
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			SweepHit hit;
			Utils::ExtractHit(m_Scene, hit, false, hitInfo.touches[i], bDrawDebug, duration);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::BoxOverlapAny(const Vec3& center, const Vec3& extent, const Vec3& rotation, const QueryFilter& filter, OverlapHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxOverlapBuffer hitInfo;
		bool bResult = BoxOverlapInternal(center, extent, rotation, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(outHit, hitInfo.block);
		}
		if (bDrawDebug)
		{
			DebugDrawUtils::DrawBox(*m_Scene, center, extent, bResult ? Vec3(0.0f, 1.0f, 0.0f) : Vec3{ 1.0f }, rotation, duration);
		}

		return bResult;
	}

	bool PhysXScene::BoxOverlapMulti(const Vec3& center, const Vec3& extent, const Vec3& rotation, const QueryFilter& filter, std::vector<OverlapHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxOverlapHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxOverlapBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = BoxOverlapInternal(center, extent, rotation, filter, hitInfo);
		if (bDrawDebug)
		{
			DebugDrawUtils::DrawBox(*m_Scene, center, extent, bResult ? Vec3(0.0f, 1.0f, 0.0f) : Vec3{ 1.0f }, rotation, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			OverlapHit hit;
			Utils::ExtractHit(hit, hitInfo.touches[i]);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::SphereOverlapAny(const Vec3& center, float radius, const QueryFilter& filter, OverlapHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxOverlapBuffer hitInfo;
		bool bResult = SphereOverlapInternal(center, radius, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(outHit, hitInfo.block);
		}
		if (bDrawDebug)
		{
			DebugDrawUtils::DrawSphere(*m_Scene, center, bResult ? Vec3(0.0f, 1.0f, 0.0f) : Vec3{ 1.0f }, radius, duration);
		}

		return bResult;
	}

	bool PhysXScene::SphereOverlapMulti(const Vec3& center, float radius, const QueryFilter& filter, std::vector<OverlapHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxOverlapHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxOverlapBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = SphereOverlapInternal(center, radius, filter, hitInfo);
		if (bDrawDebug)
		{
			DebugDrawUtils::DrawSphere(*m_Scene, center, bResult ? Vec3(0.0f, 1.0f, 0.0f) : Vec3{ 1.0f }, radius, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			OverlapHit hit;
			Utils::ExtractHit(hit, hitInfo.touches[i]);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::CapsuleOverlapAny(const Vec3& center, float radius, float height, const Vec3& rotation, const QueryFilter& filter, OverlapHit& outHit, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxOverlapBuffer hitInfo;
		bool bResult = CapsuleOverlapInternal(center, radius, height, rotation, filter, hitInfo);
		if (bResult)
		{
			Utils::ExtractHit(outHit, hitInfo.block);
		}
		if (bDrawDebug)
		{
			DebugDrawUtils::DrawCapsule(*m_Scene, center, bResult ? Vec3(0.0f, 1.0f, 0.0f) : Vec3{ 1.0f }, radius, height, rotation, 32, duration);
		}

		return bResult;
	}

	bool PhysXScene::CapsuleOverlapMulti(const Vec3& center, float radius, float height, const Vec3& rotation, const QueryFilter& filter, std::vector<OverlapHit>& outHits, bool bDrawDebug, float duration) const
	{
		ZE_PROFILE_FUNC();

		physx::PxOverlapHit hitBuffer[PHYSICS_MAX_RAYCAST_HITS];
		physx::PxOverlapBuffer hitInfo(hitBuffer, PHYSICS_MAX_RAYCAST_HITS);
		bool bResult = CapsuleOverlapInternal(center, radius, height, rotation, filter, hitInfo);
		if (bDrawDebug)
		{
			DebugDrawUtils::DrawCapsule(*m_Scene, center, bResult ? Vec3(0.0f, 1.0f, 0.0f) : Vec3{ 1.0f }, radius, height, rotation, 32, duration);
		}
		if (!bResult) return false;

		outHits.clear();
		outHits.reserve(hitInfo.getNbAnyHits());
		for (U32 i = 0; i < hitInfo.nbTouches; ++i)
		{
			OverlapHit hit;
			Utils::ExtractHit(hit, hitInfo.touches[i]);
			outHits.emplace_back(hit);
		}

		return true;
	}

	bool PhysXScene::RaycastInternal(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, physx::PxRaycastBuffer& hitInfo) const
	{
		physx::PxQueryFilterData filterData;
		filterData.data.word0 = filter.QueriesForGroup;
		filterData.flags = physx::PxQueryFlags(static_cast<U16>(filter.Type));
		if (hitInfo.maxNbTouches)
		{
			filterData.data.word1 = filter.BlockingHitGroup;
			filterData.flags |= physx::PxQueryFlag::ePREFILTER;
		}
		return m_PhysicsScene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo, physx::PxHitFlags(physx::PxHitFlag::eDEFAULT), filterData, &s_QueryFilterCallback);
	}

	bool PhysXScene::SweepInternal(const physx::PxGeometry& geometry, const Vec3& center, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const
	{
		physx::PxQueryFilterData filterData;
		filterData.data.word0 = filter.QueriesForGroup;
		filterData.flags = physx::PxQueryFlags(static_cast<U16>(filter.Type));
		if (hitInfo.maxNbTouches)
		{
			filterData.data.word1 = filter.BlockingHitGroup;
			filterData.flags |= physx::PxQueryFlag::ePREFILTER;
		}
		return m_PhysicsScene->sweep(geometry, PhysXUtils::ToPhysXTransform(center, rotation), PhysXUtils::ToPhysXVector(glm::normalize(sweepDirection)), sweepDistance, hitInfo, physx::PxHitFlags(physx::PxHitFlag::eDEFAULT), filterData, &s_QueryFilterCallback);
	}

	bool PhysXScene::BoxSweepInternal(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const
	{
		const physx::PxBoxGeometry box(extent.x * 0.5f, extent.y * 0.5f, extent.z * 0.5f);
		return SweepInternal(box, center, rotation, sweepDirection, sweepDistance, filter, hitInfo);
	}

	bool PhysXScene::SphereSweepInternal(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const
	{
		const physx::PxSphereGeometry sphere(radius);
		return SweepInternal(sphere, center, Vec3{ 0.0f }, sweepDirection, sweepDistance, filter, hitInfo);
	}

	bool PhysXScene::CapsuleSweepInternal(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const
	{
		const physx::PxCapsuleGeometry capsule(radius, height * 0.5f);
		return SweepInternal(capsule, center, rotation, sweepDirection, sweepDistance, filter, hitInfo);
	}

	bool PhysXScene::OverlapInternal(const physx::PxGeometry& geometry, const Vec3& center, const Vec3& rotation, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const
	{
		physx::PxQueryFilterData filterData;
		filterData.data.word0 = filter.QueriesForGroup;
		filterData.flags = physx::PxQueryFlags(static_cast<U16>(filter.Type));
		if (!hitInfo.maxNbTouches)
		{
			filterData.flags |= physx::PxQueryFlag::eANY_HIT; // Overlap only supports AnyHit if no touch buffer provided
		}
		return m_PhysicsScene->overlap(geometry, PhysXUtils::ToPhysXTransform(center, rotation), hitInfo, filterData);
	}

	bool PhysXScene::BoxOverlapInternal(const Vec3& center, const Vec3& extent, const Vec3& rotation, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const
	{
		const physx::PxBoxGeometry box(extent.x * 0.5f, extent.y * 0.5f, extent.z * 0.5f);
		return OverlapInternal(box, center, rotation, filter, hitInfo);
	}

	bool PhysXScene::SphereOverlapInternal(const Vec3& center, float radius, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const
	{
		const physx::PxSphereGeometry sphere(radius);
		return OverlapInternal(sphere, center, Vec3{ 0.0f }, filter, hitInfo);
	}

	bool PhysXScene::CapsuleOverlapInternal(const Vec3& center, float radius, float height, const Vec3& rotation, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const
	{
		const physx::PxCapsuleGeometry capsule(radius, height * 0.5f);
		return OverlapInternal(capsule, center, rotation, filter, hitInfo);
	}

	void PhysXScene::CreateRegions() const
	{
		physx::PxBroadPhaseCaps broadPhaseCaps;
		m_PhysicsScene->getBroadPhaseCaps(broadPhaseCaps);
		if (!broadPhaseCaps.needsPredefinedBounds) return;

		const auto& settings = PhysicsEngine::GetSettings();
		auto* regionBounds = new physx::PxBounds3[settings.WorldBoundsSubdivisions * settings.WorldBoundsSubdivisions];
		const physx::PxBounds3 globalBounds = physx::PxBounds3::centerExtents(PhysXUtils::ToPhysXVector(settings.WorldBoundsCenter), PhysXUtils::ToPhysXVector(settings.WorldBoundsExtent));
		const U32 regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, settings.WorldBoundsSubdivisions);
		for (U32 i = 0; i < regionCount; i++)
		{
			physx::PxBroadPhaseRegion region;
			region.bounds = regionBounds[i];
			m_PhysicsScene->addBroadPhaseRegion(region);
		}
		
		delete[] regionBounds;
	}

	bool PhysXScene::Advance(DeltaTime dt)
	{
		EvaluateSubSteps(dt);

		for (U32 i = 0; i < m_NumSubSteps; ++i)
		{
			m_PhysicsScene->simulate(m_SubStepTime);
			m_PhysicsScene->fetchResults(true);
		}

		return m_NumSubSteps != 0;
	}

	void PhysXScene::EvaluateSubSteps(DeltaTime dt)
	{
		if (m_AccumulatedDeltaTime > m_SubStepTime)
		{
			m_AccumulatedDeltaTime = 0.0f;
		}

		m_AccumulatedDeltaTime += dt;
		if (m_AccumulatedDeltaTime < m_SubStepTime)
		{
			m_NumSubSteps = 0;
			return;
		}

		m_NumSubSteps = glm::min(static_cast<U32>(m_AccumulatedDeltaTime / m_SubStepTime), c_MaxSubSteps);
		m_AccumulatedDeltaTime -= static_cast<float>(m_NumSubSteps) * m_SubStepTime;
	}

}
