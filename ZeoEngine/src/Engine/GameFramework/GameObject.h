#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

#define WORLD_UP_VECTOR glm::vec2({ 0.0f, 1.0f })
#define WORLD_RIGHT_VECTOR glm::vec2({ 1.0f, 0.0f })

	struct Transform
	{
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		float rotation = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };
	};

	enum class ObjectCollisionType
	{
		None,
		Box,
		Sphere,
	};

	struct CollisionData
	{
		virtual void UpdateData() = 0;

		ObjectCollisionType collisionType;
		glm::vec2 center = { 0.0f, 0.0f };
		class GameObject* ownerObject;
	};

	struct BoxCollisionData : public CollisionData
	{
	public:
		BoxCollisionData(GameObject* _ownerObject, const glm::vec2& _centerOffset, const glm::vec2& _extentsMultiplier)
			: centerOffset(_centerOffset)
			, extentsMultiplier(_extentsMultiplier)
		{
			collisionType = ObjectCollisionType::Box;
			ownerObject = _ownerObject;
			UpdateData();
		}

		virtual void UpdateData() override;

		glm::vec2 extents;
	private:
		glm::vec2 centerOffset;
		glm::vec2 extentsMultiplier;
	};

	struct SphereCollisionData : public CollisionData
	{
	public:
		SphereCollisionData(GameObject* _ownerObject, const glm::vec2& _centerOffset, float _radiusMultiplier)
			: centerOffset(_centerOffset)
			, radiusMultiplier(_radiusMultiplier)
		{
			collisionType = ObjectCollisionType::Sphere;
			ownerObject = _ownerObject;
			UpdateData();
		}

		virtual void UpdateData() override;

		float radius;
	private:
		glm::vec2 centerOffset;
		float radiusMultiplier;
	};

	// TODO: move some variables and functionalities to components
	class GameObject
	{
	public:
		GameObject() = default;
		GameObject(const Transform& transform);
		GameObject(const glm::vec3& position, float rotation, const glm::vec2& scale);

		virtual ~GameObject();

		inline const std::string& GetName() const { return m_Name; }
		inline void SetName(const std::string& name) { m_Name = name; }
		inline GameObject* GetOwner() const { return m_Owner; }
		inline void SetOwner(GameObject* owner) { m_Owner = owner; }
		inline bool IsActive() const { return m_bIsActive; }
		inline void SetActive(bool bIsActive) { m_bIsActive = bIsActive; }
		inline const Transform& GetTransform() const { return m_Transform; }
		inline void SetTransform(const Transform& transform) { m_Transform = transform; }
		inline const glm::vec2 GetPosition2D() const { return { m_Transform.position.x, m_Transform.position.y }; }
		inline const glm::vec3& GetPosition() const { return  m_Transform.position; }
		inline void SetPosition2D(const glm::vec2& position)
		{
			m_Transform.position.x = position.x;
			m_Transform.position.y = position.y;
			if (m_CollisionData)
			{
				m_CollisionData->UpdateData();
			}
		}
		/** FOr most cases, you should use SetPosition2D() for 2D rendering instead. */
		inline void SetPosition(const glm::vec3& position)
		{
			m_Transform.position = position;
			if (m_CollisionData)
			{
				m_CollisionData->UpdateData();
			}
		}
		/** For draw command, you should use radians as rotation. */
		inline const float GetRotation(bool bInRadians) const { return bInRadians ? glm::radians(m_Transform.rotation) : m_Transform.rotation; }
		inline void SetRotation(float rotation) { m_Transform.rotation = rotation; }
		inline const glm::vec2& GetScale() const { return m_Transform.scale; }
		inline void SetScale(const glm::vec2& scale)
		{
			m_Transform.scale = scale;
			if (m_CollisionData)
			{
				m_CollisionData->UpdateData();
			}
		}
		inline void SetScale(float uniformScale) { m_Transform.scale = { uniformScale, uniformScale }; }
		inline float GetSpeed() const { return m_Speed; }
		inline void SetSpeed(float speed) { m_Speed = speed; }
		inline const glm::vec2& GetVelocity() const { return m_Velocity; }
		inline ObjectCollisionType GetCollisionType() const { return m_CollisionData ? m_CollisionData->collisionType : ObjectCollisionType::None; }
		inline bool ShouldGenerateOverlapEvent() const { return m_bGenerateOverlapEvent; }
		inline void SetGenerateOverlapEvent(bool bGenerate) { m_bGenerateOverlapEvent = bGenerate; }
		inline bool IsTranslucent() const { return m_bIsTranslucent; }
		inline void SetTranslucent(bool bIsTranslucent) { m_bIsTranslucent = bIsTranslucent; }

		/**
		 * You should do initialization stuff here including loading the texture and updating IsTranslucent variable.
		 * Putting other gameplay related code here is not recommended.
		 */
		virtual void Init() {}
		virtual void BeginPlay() {}
		virtual void OnUpdate(DeltaTime dt);
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}

		/**
		 * Get forward vector based on this object's rotation.
		 * @see WORLD_UP_VECTOR
		 */
		const glm::vec2 GetForwardVector() const;
		/**
		 * Get right vector based on this object's rotation.
		 * @see WORLD_RIGHT_VECTOR
		 */
		const glm::vec2 GetRightVector() const;

		float FindLookAtRotation(const glm::vec2& sourcePosition, const glm::vec2& targetPosition);

		void TranslateTo(const glm::vec2& targetPosition);

		glm::vec2 GetRandomPositionInRange(const glm::vec2& center, const glm::vec2& extents);

		/** Reset necessary data, mostly used by ObjectPooler. */
		virtual void Reset();

		void SetBoxCollisionData(const glm::vec2& extentsMultiplier = { 1.0f, 1.0f }, const glm::vec2& centerOffset = { 0.0f, 0.0f });
		void SetSphereCollisionData(float radiusMultiplier = 1.0f, const glm::vec2& centerOffset = { 0.0f, 0.0f });

		bool IsCollisionEnabled() const { return m_CollisionData ? m_CollisionData->collisionType > ObjectCollisionType::None : false; }

		void DoCollisionTest(const std::vector<GameObject*>& objects);

		virtual void OnOverlap(GameObject* other) {}

		virtual void ApplyDamage(float damage, GameObject* target, GameObject* causer, GameObject* instigator);
		virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) {}

		void Destroy();
		virtual void OnDestroyed() {}

	private:
		bool CheckCollision(GameObject* other);
		bool CheckCollision_BB(GameObject* other);
		bool CheckCollision_BS(GameObject* boxObject, GameObject* sphereObject);
		bool CheckCollision_SS(GameObject* other);

	private:
		std::string m_Name;
		GameObject* m_Owner;
		bool m_bIsActive = true;

		Transform m_Transform;
		/** A scalar indicating how fast this object moves */
		float m_Speed = 0.0f;
		/** A vector representing current velocity of this object  */
		glm::vec2 m_Velocity{ 0.0f, 0.0f };
		CollisionData* m_CollisionData = nullptr;
		bool m_bGenerateOverlapEvent = false;

		bool bPendingDestroy = false;
		GameObject* OverlappedObject = nullptr;
		bool m_bStartMoving = false;
		glm::vec2 m_SourcePosition = { 0.0f, 0.0f }, m_TargetPosition = { 0.0f, 0.0f };
		float m_MovingDistance = 0.0f;
		float m_MovingAlpha = 0.0f;
		glm::vec2 m_LastPosition{ 0.0f, 0.0f };

		bool m_bIsTranslucent = false;
	};

}
