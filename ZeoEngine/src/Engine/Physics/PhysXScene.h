#pragma once

#include <PxQueryReport.h>

#include "Engine/Core/DeltaTime.h"
#include "Engine/Physics/PhysXActor.h"

namespace ZeoEngine {

	class PhysXActor;

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

		bool Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, RaycastHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool RaycastMulti(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, std::vector<RaycastHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool BoxSweep(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool BoxSweepMulti(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool SphereSweep(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool SphereSweepMulti(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;
		bool CapsuleSweep(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, SweepHit& outHit, bool bDrawDebug = false, float duration = 2.0f) const;
		bool CapsuleSweepMulti(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, std::vector<SweepHit>& outHits, bool bDrawDebug = false, float duration = 2.0f) const;

	private:
		void CreateRegions() const;

		bool Advance(DeltaTime dt);
		void EvaluateSubSteps(DeltaTime dt);

		bool RaycastInternal(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, physx::PxRaycastBuffer& hitInfo) const;
		bool SweepInternal(const physx::PxGeometry& geometry, const Vec3& center, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool BoxSweepInternal(const Vec3& center, const Vec3& extent, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool SphereSweepInternal(const Vec3& center, float radius, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;
		bool CapsuleSweepInternal(const Vec3& center, float radius, float height, const Vec3& rotation, const Vec3& sweepDirection, float sweepDistance, const QueryFilter& filter, physx::PxSweepBuffer& hitInfo) const;

	private:
		Scene* m_Scene = nullptr;
		physx::PxScene* m_PhysicsScene = nullptr;

		/** Map from entity ID to PhysX actor */
		std::unordered_map<UUID, Scope<PhysXActor>> m_Actors;

		float m_SubStepTime;
		float m_AccumulatedDeltaTime = 0.0f;
		U32 m_NumSubSteps = 0;
		const U32 c_MaxSubSteps = 8;
	};

}
