#include "Obstacle.h"

#include "ShooterGame.h"
#include "Level.h"
#include "RandomEngine.h"
#include "Player.h"

Obstacle::Obstacle()
	: m_ScoreAmount(1)
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

	SetRotation(GetRotation(false) + m_RotationSpeed * dt);

	if (m_Level)
	{
		SetPosition2D(GetPosition2D() - WORLD_UP_VECTOR * GetSpeed() * (float)dt);
		if (GetPosition().y < m_Level->GetLevelBounds().bottom - 0.5f)
		{
			SetActive(false);
		}
	}
}

void Obstacle::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), m_ObstacleTexture);
}

void Obstacle::Reset()
{
	Super::Reset();

	m_MaxHealth = RandomEngine::RandFloatInRange(10.0f, 50.0f);
	m_CurrentHealth = m_MaxHealth;
	m_ExplosionDamage = RandomEngine::RandFloatInRange(5.0f, 20.0f);
	m_RotationSpeed = RandomEngine::RandFloatInRange(-10.0f, 10.0f);
	SetPosition2D({ RandomEngine::RandFloatInRange(m_Level->GetLevelBounds().right - 0.5f, m_Level->GetLevelBounds().left + 0.5f), m_Level->GetLevelBounds().top });
	SetRotation(RandomEngine::RandFloatInRange(0.0f, 180.0f));
	SetScale(RandomEngine::RandFloatInRange(0.75f, 1.8f));
	SetSpeed(RandomEngine::RandFloatInRange(0.5f, 1.5f));
}

void Obstacle::TakeDamage(float damage, GameObject* causer, GameObject* instigator)
{
	Super::TakeDamage(damage, causer, instigator);

	m_CurrentHealth -= damage;
	if (m_CurrentHealth <= 0.0f)
	{
		Player* player = dynamic_cast<Player*>(instigator);
		if (player)
		{
			player->AddScore(m_ScoreAmount);
		}
		SetActive(false);
	}
}

void Obstacle::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (Player* player = dynamic_cast<Player*>(other))
	{
		ApplyDamage(m_ExplosionDamage, other, this, this);
		if (player)
		{
			player->AddScore(-m_ScoreAmount);
		}
		SetActive(false);
	}
}
