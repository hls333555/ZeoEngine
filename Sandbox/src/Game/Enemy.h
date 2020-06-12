#pragma once

#include "ZeoEngine.h"

#include "Engine/GameFramework/ObjectPooler.h"

#include "Bullet.h"

class ZeoEngine::ParticleSystem;

class Enemy : public ZeoEngine::SpriteObject
{
	using BulletPool = ZeoEngine::ObjectPooler<EnemyBullet, 3>;

public:
	Enemy();

	virtual void Init() override;
	virtual void BeginPlay() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;

	virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) override;

	virtual void OnOverlap(GameObject* other) override;

	inline void SetMaxHealth(float maxHealth) { m_MaxHealth = maxHealth; m_CurrentHealth = maxHealth; }

private:
	glm::vec2 GetRandomPositionInRange(const glm::vec2& center, const glm::vec2& extents);

	void SpawnBullet();

	void Explode();

private:
	float m_MaxHealth;
	float m_CurrentHealth;
	float m_ExplosionDamage;
	int32_t m_ScoreAmount;

	ZeoEngine::Scope<BulletPool> m_BulletPool;

	float m_ShootRate;
	bool m_bCanShoot = true;

	ZeoEngine::ParticleSystem* m_ExplosionParticle = nullptr;

	RTTR_REGISTER(Enemy, SpriteObject)
	RTTR_REGISTRATION_FRIEND
};
