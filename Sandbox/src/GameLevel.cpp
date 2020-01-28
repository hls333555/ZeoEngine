#include "GameLevel.h"

#include "Player.h"
#include "Enemy.h"

// Putting it inside header file will cause linking errors
ZeoEngine::Level* ZeoEngine::CreateLevel()
{
	return new GameLevel();
}

void GameLevel::Init()
{
	Super::Init();

	m_ObstaclePool = ZeoEngine::CreateScope<ObstaclePool>(this);

	// Spawn player ship
	m_Player = SpawnGameObject<Player>({ 0.0f, GetLevelBounds().bottom + 1.0f, 0.1f });

	// Spawn enemy ship
	DelaySpawnEnemy(5.0f);
}

void GameLevel::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

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
		glm::vec3 pos({ ZeoEngine::RandomEngine::RandFloatInRange(GetLevelBounds().right - 1.5f, GetLevelBounds().left + 1.5f), GetLevelBounds().top, 0.0f });
		glm::vec2 scale({ 1.0f, 1.0f });
		float rot = 180.0f;
		SpawnGameObject<Enemy>(pos, scale, rot);
	});
}

void GameLevel::SpawnObstacle()
{
	// "Spawn" an obstacle from pool
	Obstacle* obstacle = m_ObstaclePool->GetNextPooledObject();
}
