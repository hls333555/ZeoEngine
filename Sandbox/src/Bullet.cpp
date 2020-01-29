#include "Bullet.h"

#include "GameLevel.h"
#include "Enemy.h"
#include "Player.h"
#include "Obstacle.h"

Bullet::Bullet()
	: m_bCanPenetrate(false)
{
	SetScale({ 0.075f, 0.2f });
	SetSpeed(10.0f);
	SetBoxCollisionData();
	SetGenerateOverlapEvent(true);
}

PlayerBullet::PlayerBullet()
{
	m_Damage = 10.0f;
}

void PlayerBullet::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetPosition2D(GetPosition2D() + GetForwardVector() * GetSpeed() * (float)dt);
	if (GetPosition().y > ZeoEngine::Level::Get().GetLevelBounds().top)
	{
		SetActive(false);
	}
}

void PlayerBullet::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawQuad(GetPosition(), GetScale(), { 0.0f, 0.5f, 1.0f, 1.0f });
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
	m_Damage = 5.0f;
}

void EnemyBullet::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetPosition2D(GetPosition2D() - GetForwardVector() * GetSpeed() * (float)dt);
	if (GetPosition().y < ZeoEngine::Level::Get().GetLevelBounds().bottom)
	{
		SetActive(false);
	}
}

void EnemyBullet::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawQuad(GetPosition(), GetScale(), { 1.0f, 0.0f, 0.0f, 1.0f });
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
