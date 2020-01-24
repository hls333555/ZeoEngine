#include "Bullet.h"

#include "ShooterGame.h"
#include "Level.h"

Bullet::Bullet()
	: m_Damage(10.0f)
	, m_bCanPenetrate(false)
{
	SetScale({ 0.075f, 0.2f });
	SetSpeed(10.0f);
	SetBoxCollisionData();
	SetGenerateOverlapEvent(true);
}

void Bullet::Init()
{
	Super::Init();

	m_Level = GetLevel();
}

void Bullet::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	if (m_Level)
	{
		SetPosition({ GetPosition().x, GetPosition().y + GetSpeed() * dt, GetPosition().z });
		if (GetPosition().y > m_Level->GetLevelBounds().top)
		{
			SetActive(false);
		}
	}
}

void Bullet::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawQuad(GetPosition(), GetScale(), { 1.0f, 0.5f, 0.0f, 1.0f });
}

void Bullet::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (dynamic_cast<Obstacle*>(other))
	{
		ApplyDamage(other, m_Damage);
		if (!m_bCanPenetrate)
		{
			SetActive(false);
		}
	}
}
