#pragma once

#include "GameObject.h"

#include "ObjectPooler.h"

class Level;
class PlayerBullet;

class Player : public GameObject
{
public:
	Player();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;

	virtual void TakeDamage(GameObject* source, float damage) override;

private:
	void SpawnBullet();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_PlayerTexture;

	float m_MaxHealth;
	float m_CurrentHealth;

	typedef ObjectPooler<PlayerBullet, 5> BulletPool;
	ZeoEngine::Scope<BulletPool> m_BulletPool;

	float m_ShootRate;
	bool m_bCanShoot = true;

	Level* m_Level;
};
