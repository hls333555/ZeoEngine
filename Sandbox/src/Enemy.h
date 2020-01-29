#pragma once

#include "ZeoEngine.h"

#include "Engine/GameFramework/ObjectPooler.h"

#include "Bullet.h"

class ZeoEngine::ParticleSystem;

class Enemy : public ZeoEngine::GameObject
{
public:
	Enemy();

	virtual void Init() override;
	virtual void BeginPlay() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) override;

	virtual void OnDestroyed() override;

	virtual void OnOverlap(GameObject* other) override;

	inline void SetMaxHealth(float maxHealth) { m_MaxHealth = maxHealth; m_CurrentHealth = maxHealth; }

private:
	void SpawnBullet();

	void Explode();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_EnemyTexture;

	float m_MaxHealth;
	float m_CurrentHealth;
	float m_ExplosionDamage;
	int32_t m_ScoreAmount;

	typedef ZeoEngine::ObjectPooler<EnemyBullet, 3> BulletPool;
	ZeoEngine::Scope<BulletPool> m_BulletPool;

	float m_ShootRate;
	bool m_bCanShoot = true;

	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ExplosionTexture;
	ZeoEngine::ParticleSystem* m_ExplosionParticle;

};
