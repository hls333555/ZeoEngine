#pragma once

#include "GameObject.h"

#include "ObjectPooler.h"

class Level;
struct ImFont;
class PlayerBullet;

class Player : public GameObject
{
public:
	Player();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;

	virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) override;

	inline uint32_t GetScore() const { return m_Score; }
	void AddScore(int32_t deltaScore) { m_Score = glm::clamp((int32_t)m_Score + deltaScore, 0, INT_MAX); }

private:
	void SpawnBullet();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_PlayerTexture;

	float m_MaxHealth;
	float m_CurrentHealth;
	uint32_t m_Score;

	typedef ObjectPooler<PlayerBullet, 5> BulletPool;
	ZeoEngine::Scope<BulletPool> m_BulletPool;

	float m_ShootRate;
	bool m_bCanShoot = true;

	Level* m_Level;
	ImFont* m_Font;
};
