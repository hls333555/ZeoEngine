#include "Obstacle.h"

#include "ShooterGame.h"
#include "Level.h"
#include "RandomEngine.h"
#include "Player.h"

Obstacle::Obstacle()
{
	SetSphereCollisionData(0.75f);
	SetGenerateOverlapEvent(true);
}

void Obstacle::Init()
{
	Super::Init();

	m_Level = GetLevel();

	m_ObstacleTexture = ZeoEngine::Texture2D::Create("assets/textures/Obstacle.png");
	SetTranslucent(m_ObstacleTexture->HasAlpha());
}

void Obstacle::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetRotation(GetRotation() + m_RotationSpeed * dt);

	if (m_Level)
	{
		SetPosition({ GetPosition().x, GetPosition().y - GetSpeed() * dt, GetPosition().z });
		if (GetPosition().y < m_Level->GetLevelBounds().bottom - 0.5f)
		{
			SetActive(false);
		}
	}
}

void Obstacle::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(), m_ObstacleTexture);
}

void Obstacle::Reset()
{
	Super::Reset();

	m_MaxHealth = RandomEngine::RandFloatInRange(10.0f, 50.0f);
	m_CurrentHealth = m_MaxHealth;
	m_Damage = RandomEngine::RandFloatInRange(5.0f, 20.0f);
	SetPosition({ RandomEngine::RandFloatInRange(m_Level->GetLevelBounds().right - 0.5f, m_Level->GetLevelBounds().left + 0.5f), m_Level->GetLevelBounds().top, 0.0f });
	SetRotation(RandomEngine::RandFloatInRange(0.0f, 360.0f));
	SetScale(RandomEngine::RandFloatInRange(0.75f, 1.8f));
	SetSpeed(RandomEngine::RandFloatInRange(0.5f, 1.5f));
	SetRotationSpeed(RandomEngine::RandFloatInRange(-0.5f, 0.5f));
}

void Obstacle::TakeDamage(GameObject* source, float damage)
{
	Super::TakeDamage(source, damage);

	m_CurrentHealth -= damage;
	if (m_CurrentHealth <= 0.0f)
	{
		SetActive(false);
	}
}

void Obstacle::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (dynamic_cast<Player*>(other))
	{
		ApplyDamage(other, m_Damage);
		SetActive(false);
	}
}
