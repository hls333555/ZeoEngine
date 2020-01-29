#include "GameLevel.h"

#include "Player.h"
#include "Enemy.h"

void GameLevel::Init()
{
	m_ObstaclePool = ZeoEngine::CreateScope<ObstaclePool>();

	auto& level = ZeoEngine::Level::Get();
	// Spawn player ship
	m_Player = level.SpawnGameObject<Player>({ 0.0f, level.GetLevelBounds().bottom + 1.0f, 0.1f });

	// Spawn enemy ship
	DelaySpawnEnemy(5.0f);
}

void GameLevel::OnUpdate(ZeoEngine::DeltaTime dt)
{
	// Spawn an obstacle every random intervals
	if (m_bShouldSpawnObstacle)
	{
		m_bShouldSpawnObstacle = false;
		float obstacleSpawnRate = ZeoEngine::RandomEngine::RandFloatInRange(1.0f, 4.0f);
		ZeoEngine::GetTimerManager()->SetTimer(obstacleSpawnRate, [&]() {
			m_bShouldSpawnObstacle = true;
		});

		SpawnObstacle();
	}
}

void GameLevel::DelaySpawnEnemy(float delay)
{
	ZeoEngine::GetTimerManager()->SetTimer(delay, [&]() {
		auto& level = ZeoEngine::Level::Get();
		glm::vec3 pos({ ZeoEngine::RandomEngine::RandFloatInRange(level.GetLevelBounds().right - 1.5f, level.GetLevelBounds().left + 1.5f), level.GetLevelBounds().top, 0.0f });
		glm::vec2 scale({ 1.0f, 1.0f });
		float rot = 180.0f;
		level.SpawnGameObject<Enemy>(pos, scale, rot);
	});
}

void GameLevel::SpawnObstacle()
{
	// "Spawn" an obstacle from pool
	Obstacle* obstacle = m_ObstaclePool->GetNextPooledObject();
}
