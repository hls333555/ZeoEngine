#pragma once

#include "Engine/Asset/Asset.h"

#define PHYSICS_MAX_CONTACT_POINTS 16
#define PHYSICS_MAX_RAYCAST_HITS 256 // TODO: May need to figure out a proper value

namespace ZeoEngine {

	class PhysXColliderShapeBase;
	class PhysXCharacterController;

	enum class BroadphaseType
	{
		SweepAndPrune = 0,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch = 0,
		OneDirectional,
		TwoDirectional
	};

	enum class SolverType
	{
		ProjectedGaussSeidel,
		TemporalGaussSeidel
	};

	enum class ActorLockFlag
	{
		TranslationX = ZE_BIT(0), TranslationY = ZE_BIT(1), TranslationZ = ZE_BIT(2), Translation = TranslationX | TranslationY | TranslationZ,
		RotationX = ZE_BIT(3), RotationY = ZE_BIT(4), RotationZ = ZE_BIT(5), Rotation = RotationX | RotationY | RotationZ
	};

	enum class ForceMode
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class ColliderType
	{
		Box, Sphere, Capsule, ConvexMesh, TriangleMesh
	};

	struct ContactInfo
	{
		Vec3 ContactPosition;
		Vec3 ContactNormal;
		float Separation;
		AssetHandle PhysicsMaterialAssets[2];
	};

	struct CollisionInfo
	{
		UUID OtherEntity = 0;
		PhysXColliderShapeBase* OtherCollider = nullptr;
		U32 NumContacts = 0;
		ContactInfo Contacts[PHYSICS_MAX_CONTACT_POINTS];
	};

	struct CharacterControllerHit
	{
		UUID HitEntity = 0;
		PhysXColliderShapeBase* HitCollider = nullptr; // Not available when colliding with other character controller
		PhysXCharacterController* HitController = nullptr; // Only available when colliding with other character controller
		Vec3 Position;
		Vec3 Normal;
		Vec3 MoveDirection;
		float MoveLength;
	};

	struct OverlapHit
	{
		UUID HitEntity = 0;
		PhysXColliderShapeBase* HitCollider = nullptr;
	};

	struct RaycastHit : public OverlapHit
	{
		bool bIsBlockingHit = false;
		Vec3 Position;
		Vec3 Normal;
		float Distance;
	};

	using SweepHit = RaycastHit;

	enum class QueryFlag : U16
	{
		Static = ZE_BIT(0), Dynamic = ZE_BIT(1), Both = Static | Dynamic
	};

	struct QueryFilter
	{
		QueryFlag Type = QueryFlag::Both;
		I32 QueriesFor = 0; // Bitfield that contains a set of layer bits for which query targets
		I32 BlockingHitLayerMask = 0; // Bitfield that contains a set of layer bits which are considered to be blocking hits. Only used for multi non-overlap queries
	};

	enum class CharacterControllerCollisionFlag
	{
		Sides = ZE_BIT(0),
		Above = ZE_BIT(1),
		Below = ZE_BIT(2)
	};

}
