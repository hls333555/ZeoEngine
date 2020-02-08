#pragma once

#include "ZeoEngine.h"

class ZeoEngine::ParticleSystem;

class Obstacle : public ZeoEngine::SpriteObject
{
public:
	Obstacle();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;

	virtual void Reset() override;

	virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) override;

	virtual void OnOverlap(GameObject* other) override;

	inline void SetRotationSpeed(float rotationSpeed) { m_RotationSpeed = rotationSpeed; }

	void Explode();

private:
	float m_RotationSpeed = 0.0f;

	float m_MaxHealth;
	float m_CurrentHealth;
	float m_ExplosionDamage;
	int32_t m_ScoreAmount;

	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ExplosionTexture;
	ZeoEngine::ParticleSystem* m_ExplosionParticle;

	RTTR_REGISTER(Obstacle, SpriteObject)
};
