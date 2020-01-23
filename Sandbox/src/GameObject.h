#pragma once

#include "ZeoEngine.h"

#define Super __super

struct Transform
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	float rotation = 0.0f;
	glm::vec2 scale = { 1.0f, 1.0f };
};

class GameObject
{
public:
	GameObject() = default;
	GameObject(const Transform& transform);
	GameObject(const glm::vec3& position, float rotation, const glm::vec2& scale);

	virtual ~GameObject() = default;

	inline const std::string& GetName() const { return m_Name; }
	inline void SetName(const std::string& name) { m_Name = name; }
	inline bool IsActive() const { return m_bIsActive; }
	inline void SetActive(bool bIsActive) { m_bIsActive = bIsActive; }
	inline const Transform& GetTransform() const { return m_Transform; }
	inline void SetTransform(const Transform& transform) { m_Transform = transform; }
	inline glm::vec3& GetPosition() { return  m_Transform.position; }
	inline void SetPosition(const glm::vec3& position) { m_Transform.position = position; }
	inline float& GetRotation() { return m_Transform.rotation; }
	inline void SetRotation(float rotation) { m_Transform.rotation = rotation; }
	inline glm::vec2& GetScale() { return m_Transform.scale; }
	inline void SetScale(const glm::vec2& scale) { m_Transform.scale = scale; }
	inline void SetScale(float uniformScale) { m_Transform.scale = { uniformScale, uniformScale }; }
	inline float GetSpeed() const { return m_Speed; }
	inline void SetSpeed(float speed) { m_Speed = speed; }
	inline bool IsTranslucent() const { return m_bIsTranslucent; }
	inline void SetTranslucent(bool bIsTranslucent) { m_bIsTranslucent = bIsTranslucent; }

	virtual void Init() {}
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) {}
	virtual void OnRender() {}
	virtual void OnImGuiRender() {}

	void Destroy();

private:
	std::string m_Name;
	bool m_bIsActive = true;

	Transform m_Transform;
	float m_Speed = 0.0f;

	bool m_bIsTranslucent = false;
};
