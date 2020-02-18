#include "Bullet.h"

#include "Enemy.h"
#include "Player.h"
#include "Obstacle.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<Bullet>("Bullet")
		(
			metadata(ClassMeta::Abstract, true)
		)
		.constructor(&Bullet::SpawnGameObject, policy::ctor::as_raw_ptr);

	registration::class_<PlayerBullet>("PlayerBullet")
		.constructor(&PlayerBullet::SpawnGameObject, policy::ctor::as_raw_ptr);

	registration::class_<EnemyBullet>("EnemyBullet")
		.constructor(&EnemyBullet::SpawnGameObject, policy::ctor::as_raw_ptr);
}

Bullet::Bullet()
	: m_bCanPenetrate(false)
{
	SetScale({ 0.075f, 0.2f });
	SetSpeed(10.0f);
	SetCollisionType(ZeoEngine::ObjectCollisionType::Box);
	SetGenerateOverlapEvents(true);
}

PlayerBullet::PlayerBullet()
{
	SetTintColor({ 0.0f, 0.5f, 1.0f, 1.0f });
	m_Damage = 10.0f;
}

void PlayerBullet::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetPosition2D(GetPosition2D() + GetForwardVector2D() * GetSpeed() * (float)dt);
	if (GetPosition().y > ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Top)
	{
		SetActive(false);
	}
}

void PlayerBullet::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (dynamic_cast<Obstacle*>(other) || dynamic_cast<Enemy*>(other))
	{
		ApplyDamage(m_Damage, other, this, GetOwner());
		if (!m_bCanPenetrate)
		{
			SetActive(false);
		}
	}
}

EnemyBullet::EnemyBullet()
{
	SetTintColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	m_Damage = 5.0f;
}

void EnemyBullet::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetPosition2D(GetPosition2D() - GetForwardVector2D() * GetSpeed() * (float)dt);
	if (GetPosition().y < ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Bottom)
	{
		SetActive(false);
	}
}

void EnemyBullet::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (dynamic_cast<Player*>(other))
	{
		ApplyDamage(m_Damage, other, this, GetOwner());
		if (!m_bCanPenetrate)
		{
			SetActive(false);
		}
	}
}
