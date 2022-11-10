#pragma once

#include "Engine/Core/DeltaTime.h"
#include "Engine/Physics/PhysXActor.h"

namespace ZeoEngine {

	class PhysXActor;

	class PhysXScene
	{
	public:
		PhysXScene();
		// TODO: Scene context require this to be move constructible
		PhysXScene(PhysXScene&&) = default;
		~PhysXScene();

		void Simulate(DeltaTime dt);

		PhysXActor* GetActor(Entity entity) const;
		PhysXActor* CreateActor(Entity entity);
		void DestroyActor(Entity entity);

	private:
		bool Advance(DeltaTime dt);
		void EvaluateSubSteps(DeltaTime dt);

	private:
		physx::PxScene* m_PhysicsScene = nullptr;

		/** Map from entity ID to PhysX actor */
		std::unordered_map<UUID, Scope<PhysXActor>> m_Actors;

		float m_SubStepTime;
		float m_AccumulatedDeltaTime = 0.0f;
		U32 m_NumSubSteps = 0;
		const U32 c_MaxSubSteps = 8;
	};

}
