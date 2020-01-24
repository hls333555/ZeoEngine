#pragma once

#include "ZeoEngine.h"

#define Super __super

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

class GameObject
{
public:
	GameObject() = default;
	GameObject(const Transform& transform);
	GameObject(const glm::vec3& position, float rotation, const glm::vec2& scale);

	virtual ~GameObject();

	inline const std::string& GetName() const { return m_Name; }
	inline void SetName(const std::string& name) { m_Name = name; }
	inline bool IsActive() const { return m_bIsActive; }
	inline void SetActive(bool bIsActive) { m_bIsActive = bIsActive; }
	inline const Transform& GetTransform() const { return m_Transform; }
	inline void SetTransform(const Transform& transform) { m_Transform = transform; }
	inline const glm::vec3& GetPosition() const { return  m_Transform.position; }
	inline void SetPosition(const glm::vec3& position)
	{
		m_Transform.position = position;
		if (m_CollisionData)
		{
			m_CollisionData->UpdateData();
		}
	}
	inline const float GetRotation() const { return m_Transform.rotation; }
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
	inline ObjectCollisionType GetCollisionType() const { return m_CollisionData ? m_CollisionData->collisionType : ObjectCollisionType::None; }
	inline bool ShouldGenerateOverlapEvent() const { return m_bGenerateOverlapEvent; }
	inline void SetGenerateOverlapEvent(bool bGenerate) { m_bGenerateOverlapEvent = bGenerate; }
	inline bool IsTranslucent() const { return m_bIsTranslucent; }
	inline void SetTranslucent(bool bIsTranslucent) { m_bIsTranslucent = bIsTranslucent; }

	virtual void Init() {}
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) {}
	virtual void OnRender() {}
	virtual void OnImGuiRender() {}

	/** Reset necessary data, mostly used by ObjectPooler. */
	virtual void Reset();

	void SetBoxCollisionData(const glm::vec2& extentsMultiplier = { 1.0f, 1.0f }, const glm::vec2& centerOffset = { 0.0f, 0.0f });
	void SetSphereCollisionData(float radiusMultiplier = 1.0f, const glm::vec2& centerOffset = { 0.0f, 0.0f });

	bool IsCollisionEnabled() const { return m_CollisionData ? m_CollisionData->collisionType > ObjectCollisionType::None : false; }

	void DoCollisionTest(const std::vector<GameObject*>& objects);

	virtual void OnOverlap(GameObject* other) {}

	virtual void ApplyDamage(GameObject* target, float damage);
	virtual void TakeDamage(GameObject* source, float damage) {}

	void Destroy();

private:
	bool CheckCollision(GameObject* other);
	bool CheckCollision_BB(GameObject* other);
	bool CheckCollision_BS(GameObject* boxObject, GameObject* sphereObject);
	bool CheckCollision_SS(GameObject* other);

private:
	std::string m_Name;
	bool m_bIsActive = true;

	Transform m_Transform;
	float m_Speed = 0.0f;
	CollisionData* m_CollisionData = nullptr;
	bool m_bGenerateOverlapEvent = false;

	GameObject* OverlappedObject = nullptr;

	bool m_bIsTranslucent = false;
};
