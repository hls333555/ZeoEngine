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

		bool Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, RaycastHit* outHit, bool bDrawDebug = false, float duration = 2.0f);
		bool RaycastMulti(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, std::vector<RaycastHit>& outHits, bool bDrawDebug = false, float duration = 2.0f);

	private:
		void CreateRegions() const;

		bool Advance(DeltaTime dt);
		void EvaluateSubSteps(DeltaTime dt);

		physx::PxRaycastBuffer RaycastMultiInternal(const Vec3& origin, const Vec3& direction, float maxDistance, const QueryFilter& filter, physx::PxRaycastHit* hitBuffer, U32 maxHits);

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
