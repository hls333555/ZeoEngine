#pragma once

#include "GameObject.h"

class Level;

class Obstacle : public GameObject
{
public:
	Obstacle();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void Reset() override;

	virtual void TakeDamage(GameObject* source, float damage) override;

	virtual void OnOverlap(GameObject* other) override;

	inline void SetRotationSpeed(float rotationSpeed) { m_RotationSpeed = rotationSpeed; }

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ObstacleTexture;

	float m_RotationSpeed = 0.0f;

	float m_MaxHealth;
	float m_CurrentHealth;
	float m_ExplosionDamage;

	Level* m_Level;

};
