#include "ZEpch.h"
#include "Engine/Physics/PhysXScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Physics/PhysXEngine.h"
#include "Engine/Physics/PhysXUtils.h"

namespace ZeoEngine {

	PhysXScene::PhysXScene()
	{
		auto& physics = PhysXEngine::GetPhysics();
		const auto& settings = PhysicsEngine::GetSettings();
		m_SubStepTime = settings.FixedDeltaTime;
		physx::PxSceneDesc sceneDesc(physics.getTolerancesScale());
		sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader; // TODO:
		sceneDesc.cpuDispatcher = PhysXEngine::GetCPUDispatcher();

		ZE_CORE_ASSERT(sceneDesc.isValid());

		m_PhysicsScene = physics.createScene(sceneDesc);
	}

	PhysXScene::~PhysXScene()
	{
		m_Actors.clear();

		m_PhysicsScene->release();
		m_PhysicsScene = nullptr;
	}

	void PhysXScene::Simulate(DeltaTime dt)
	{
		if (Advance(dt))
		{
			
		}
	}

	Ref<PhysXActor> PhysXScene::GetActor(Entity entity) const
	{
		const auto it = m_Actors.find(entity.GetUUID());
		if (it == m_Actors.end()) return nullptr;
		return it->second;
	}

	Ref<PhysXActor> PhysXScene::CreateActor(Entity entity)
	{
		auto foundActor = GetActor(entity);
		if (foundActor) return foundActor;

		auto actor = CreateRef<PhysXActor>(entity);
		// TODO: Entity hierarchy
		if (entity.HasComponent<RigidBodyComponent>())
		{
			
		}
		m_Actors.emplace(entity.GetUUID(), actor);
		m_PhysicsScene->addActor(actor->GetRigidActor());
		return actor;
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
