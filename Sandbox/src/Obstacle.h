#pragma once

#include "GameObject.h"

class Level;
class ParticleSystem;

class Obstacle : public GameObject
{
public:
	Obstacle();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void Reset() override;

	virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) override;

	virtual void OnOverlap(GameObject* other) override;

	inline void SetRotationSpeed(float rotationSpeed) { m_RotationSpeed = rotationSpeed; }

	void Explode();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ObstacleTexture;

	float m_RotationSpeed = 0.0f;

	float m_MaxHealth;
	float m_CurrentHealth;
	float m_ExplosionDamage;
	int32_t m_ScoreAmount;

	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ExplosionTexture;
	ParticleSystem* m_ExplosionParticle;

	Level* m_Level;

};
