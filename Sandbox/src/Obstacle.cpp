#include "Obstacle.h"

#include "ShooterGame.h"
#include "Level.h"

Obstacle::Obstacle()
	: MaxHealth(20.0f)
	, CurrentHealth(20.0f)
{
	SetScale({ 1.0f, 1.0f });
	SetSpeed(1.0f);
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

	if (m_Level)
	{
		GetPosition().y -= GetSpeed() * (float)dt;
		if (GetPosition().y < m_Level->GetLevelBounds().bottom)
		{
			Destroy();
		}
	}
}

void Obstacle::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawQuad(GetPosition(), GetScale(), m_ObstacleTexture);
}
