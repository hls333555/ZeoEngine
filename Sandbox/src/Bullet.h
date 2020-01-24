#pragma once

#include "GameObject.h"

class Level;

class Bullet : public GameObject
{
public:
	Bullet();

	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnRender() override;

	virtual void OnOverlap(GameObject* other) override;

private:
	Level* m_Level;

	float m_Damage;
	bool m_bCanPenetrate;
};