#pragma once

#include "ZeoEngine.h"

class GameLevel;

class Bullet : public ZeoEngine::GameObject
{
public:
	Bullet();

protected:
	GameLevel* m_Level;

	float m_Damage = 0.0f;
	bool m_bCanPenetrate;

	RTTR_REGISTER(Bullet, GameObject)
};

class PlayerBullet : public Bullet
{
public:
	PlayerBullet();

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void OnOverlap(GameObject* other) override;

	RTTR_REGISTER(PlayerBullet, Bullet)
};

class EnemyBullet : public Bullet
{
public:
	EnemyBullet();

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void OnOverlap(GameObject* other) override;

	RTTR_REGISTER(EnemyBullet, Bullet)
};
