#pragma once

#include <PxQueryReport.h>

#include "Engine/Core/DeltaTime.h"
#include "Engine/Physics/PhysXActor.h"

namespace physx
{
	class PxGeometry;
	class PxControllerManager;
}

namespace ZeoEngine {

	class PhysXActor;
	class PhysXCharacterController;

	class PhysXScene
	{
	public:
		explicit PhysXScene(Scene* scene);
		// TODO: Scene context require this to be move constructible
		PhysXScene(PhysXScene&&) = default;
		~PhysXScene();

		void Simulate(DeltaTime dt);

		PhysXActor* GetActor(Entity entity) const;
		PhysXActor* CreateActor(Entity entity);
		void DestroyActor(Entity entity);

		PhysXCharacterController* GetCharacterController(Entity entity) const;
		PhysXCharacterController* CreateCharacterController(Entity entity);
		void DestroyCharacterController(Entity entity);

		Vec3 GetGravity() const;
		void SetGravity(const Vec3& gravity) const;

		bool Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, RaycastHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool RaycastMulti(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, std::vector<RaycastHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool BoxSweep(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool BoxSweepMulti(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool SphereSweep(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool SphereSweepMulti(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool CapsuleSweep(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool CapsuleSweepMulti(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool BoxOverlapAny(const Vec3& center, const Vec3& extent, const Vec3& rotation, const QueryFilter& filter, OverlapHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool BoxOverlapMulti(const Vec3& center, const Vec3& extent, const Vec3& rotation, const QueryFilter& filter, std::vector<OverlapHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool SphereOverlapAny(const Vec3& center, float radius, const QueryFilter& filter, OverlapHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool SphereOverlapMulti(const Vec3& center, float radius, const QueryFilter& filter, std::vector<OverlapHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool CapsuleOverlapAny(const Vec3& center, float radius, float height, const Vec3& rotation, const QueryFilter& filter, OverlapHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool CapsuleOverlapMulti(const Vec3& center, float radius, float height, const Vec3& rotation, const QueryFilter& filter, std::vector<OverlapHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;

	private:
		void CreateRegions() const;

		bool Advance(DeltaTime dt);
		void EvaluateSubSteps(DeltaTime dt);

		bool RaycastInternal(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, physx::PxRaycastBuffer& hitInfo) const;
		bool SweepInternal(const physx::PxGeometry& geometry, const Vec3& center, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool BoxSweepInternal(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool SphereSweepInternal(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool CapsuleSweepInternal(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool OverlapInternal(const physx::PxGeometry& geometry, const Vec3& center, const Vec3& rotation, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const;
		bool BoxOverlapInternal(const Vec3& center, const Vec3& extent, const Vec3& rotation, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const;
		bool SphereOverlapInternal(const Vec3& center, float radius, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const;
		bool CapsuleOverlapInternal(const Vec3& center, float radius, float height, const Vec3& rotation, const QueryFilter& filter, physx::PxOverlapBuffer& hitInfo) const;

	private:
		Scene* m_Scene = nullptr;
		physx::PxScene* m_PhysicsScene = nullptr;
		physx::PxControllerManager* m_PhysicsControllerManager;

		std::vector<Scope<PhysXActor>> m_Actors;
		std::vector<Scope<PhysXCharacterController>> m_CharacterControllers;

		float m_SubStepTime;
		float m_AccumulatedDeltaTime = 0.0f;
		U32 m_NumSubSteps = 0;
		const U32 c_MaxSubSteps = 8;
	};

}
