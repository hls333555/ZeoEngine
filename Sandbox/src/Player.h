#pragma once

#include "GameObject.h"

#include "ObjectPooler.h"

class Level;
class Bullet;

class Player : public GameObject
{
public:
	Player();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ShipTexture;

	float m_ShootInterval;
	float m_MaxHealth;
	float m_CurrentHealth;

	ZeoEngine::Scope<ObjectPooler<Bullet, 10>> m_BulletPool;

	bool m_bCanShoot = true;
	float m_Time = 0.0f, m_ShootTime = 0.0f;

	Level* m_Level;
};
