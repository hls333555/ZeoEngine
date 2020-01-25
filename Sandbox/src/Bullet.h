#pragma once

#include "GameObject.h"

class Level;

class Bullet : public GameObject
{
public:
	Bullet();

	virtual void Init() override;

protected:
	Level* m_Level;

	float m_Damage = 0.0f;
	bool m_bCanPenetrate;
};

class PlayerBullet : public Bullet
{
public:
	PlayerBullet();

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void OnOverlap(GameObject* other) override;
};

class EnemyBullet : public Bullet
{
public:
	EnemyBullet();

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void OnOverlap(GameObject* other) override;
};
