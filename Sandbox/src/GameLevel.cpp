#include "GameLevel.h"

#include "Player.h"
#include "Enemy.h"

void GameLevel::Init()
{
	m_ObstaclePool = ZeoEngine::CreateScope<ObstaclePool>();

	// Spawn player ship
	m_Player = ZeoEngine::Level::Get().SpawnGameObject<Player>({ 0.0f, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Bottom + 1.0f, 0.1f });

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
		const auto& cameraBounds = ZeoEngine::GetActiveGameCamera()->GetCameraBounds();
		glm::vec3 pos({ ZeoEngine::RandomEngine::RandFloatInRange(cameraBounds.Right - 1.5f, cameraBounds.Left + 1.5f), cameraBounds.Top, 0.0f });
		glm::vec2 scale({ 1.0f, 1.0f });
		float rot = 180.0f;
		ZeoEngine::Level::Get().SpawnGameObject<Enemy>(pos, scale, rot);
	});
}

void GameLevel::SpawnObstacle()
{
	// "Spawn" an obstacle from pool
	Obstacle* obstacle = m_ObstaclePool->GetNextPooledObject();
}
