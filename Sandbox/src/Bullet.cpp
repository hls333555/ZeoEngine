#include "Bullet.h"

#include "ShooterGame.h"
#include "Level.h"

Bullet::Bullet()
{
	SetScale({ 0.075f, 0.2f });
	SetSpeed(10.0f);
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
		GetPosition().y += GetSpeed() * (float)dt;
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
