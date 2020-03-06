#pragma once

#include "ZeoEngine.h"

#include "Engine/GameFramework/ObjectPooler.h"

#include "Bullet.h"

class ZeoEngine::ParticleSystem;

class Player : public ZeoEngine::SpriteObject
{
	using BulletPool = ZeoEngine::ObjectPooler<PlayerBullet, 5>;

public:
	Player();

	virtual void Init() override;
	virtual void BeginPlay() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnImGuiRender() override;

	virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) override;

	inline uint32_t GetScore() const { return m_Score; }
	void AddScore(int32_t deltaScore) { m_Score = glm::clamp((int32_t)m_Score + deltaScore, 0, INT_MAX); }

private:
	void SpawnBullet();

	void Explode();

private:
	float m_MaxHealth;
	float m_CurrentHealth;
	uint32_t m_Score;

	ZeoEngine::Scope<BulletPool> m_BulletPool;

	float m_ShootRate;
	bool m_bCanShoot = true;

	/** NOTE: Always initialize ParticleSystem* variables with nullptr! */
	ZeoEngine::ParticleSystem* m_ShipFlameParticle = nullptr, *m_SpawnedShipFlameParticle = nullptr;
	ZeoEngine::ParticleSystem* m_ExplosionParticle = nullptr;

	RTTR_REGISTER(Player, SpriteObject)
	RTTR_REGISTRATION_FRIEND
};
