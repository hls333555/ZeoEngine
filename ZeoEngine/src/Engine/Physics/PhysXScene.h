#pragma once

#include "Engine/Core/DeltaTime.h"
#include "Engine/Physics/PhysXActor.h"

namespace ZeoEngine {

	class PhysXScene
	{
	public:
		PhysXScene();
		~PhysXScene();

		void Simulate(DeltaTime dt);

		Ref<PhysXActor> GetActor(Entity entity) const;
		Ref<PhysXActor> CreateActor(Entity entity);

	private:
		bool Advance(DeltaTime dt);
		void EvaluateSubSteps(DeltaTime dt);

	private:
		physx::PxScene* m_PhysicsScene = nullptr;

		/** Map from entity ID to PhysX actor */
		std::unordered_map<UUID, Ref<PhysXActor>> m_Actors;

		float m_SubStepTime;
		float m_AccumulatedDeltaTime = 0.0f;
		U32 m_NumSubSteps = 0;
		const U32 c_MaxSubSteps = 8;
	};

}
