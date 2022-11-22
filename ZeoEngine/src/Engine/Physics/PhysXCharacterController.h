#pragma once

#include <characterkinematic/PxController.h>

#include "Engine/Physics/PhysicsTypes.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	struct CharacterControllerComponent;

	class PhysXCharacterController
	{
		friend class PhysXScene;

	public:
		PhysXCharacterController(Entity entity, physx::PxControllerManager* controllerManager);
		~PhysXCharacterController();

		Entity GetEntity() const { return m_Entity; }

		bool IsGrounded() const { return m_CollisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN; }
		CharacterControllerCollisionFlag GetCharacterControllerCollisionFlags() const { return static_cast<CharacterControllerCollisionFlag>(static_cast<U8>(m_CollisionFlags)); }

		Vec3 GetTranslation() const;
		/** This is a 'teleport' function, it does not check for collisions. */
		void SetTranslation(const Vec3& translation) const;
		Vec3 GetFootPosition() const;
		/** This is a 'teleport' function, it does not check for collisions. */
		void SetFootPosition(const Vec3& position) const;
		Vec3 GetUpVector() const;
		void SetUpVector(const Vec3& up) const;

		void Move(const Vec3& displacement);

	private:
		void OnUpdate(DeltaTime dt);

		void SetFilterData(const CharacterControllerComponent& controllerComp);
		void SynchronizeTransform();

	private:
		Entity m_Entity;

		physx::PxController* m_Controller = nullptr;
		physx::PxFilterData m_FilterData;
		physx::PxControllerFilters m_ControllerFilters;
		Vec3 m_Displacement{ 0.0f };
		physx::PxControllerCollisionFlags m_CollisionFlags;
		float m_MinMoveDistance = 0.0f;
		float m_SpeedDown = 0.0f;
	};
	
}
