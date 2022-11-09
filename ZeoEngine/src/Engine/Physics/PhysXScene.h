#pragma once

#include <PxPhysicsAPI.h>

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class PhysXScene
	{
	public:
		PhysXScene();
		~PhysXScene();

		void Simulate(DeltaTime dt);

	private:
		bool Advance(DeltaTime dt);
		void EvaluateSubSteps(DeltaTime dt);

	private:
		physx::PxScene* m_PhysicsScene = nullptr;

		float m_SubStepTime;
		float m_AccumulatedDeltaTime = 0.0f;
		U32 m_NumSubSteps = 0;
		const U32 c_MaxSubSteps = 8;
	};

}
