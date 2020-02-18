#include "GameManager.h"

#include "Enemy.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<GameManager>("GameManager")
		.constructor(&GameManager::SpawnGameObject, policy::ctor::as_raw_ptr);
}

void GameManager::BeginPlay()
{
	Super::BeginPlay();

	m_ObstaclePool = ZeoEngine::CreateScope<ObstaclePool>(this);

	// Spawn enemy ship
	DelaySpawnEnemy(5.0f);
}

void GameManager::OnUpdate(ZeoEngine::DeltaTime dt)
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

void GameManager::DelaySpawnEnemy(float delay)
{
	ZeoEngine::GetTimerManager()->SetTimer(delay, [&]() {
		const auto& cameraBounds = ZeoEngine::GetActiveGameCamera()->GetCameraBounds();
		glm::vec3 pos({ ZeoEngine::RandomEngine::RandFloatInRange(cameraBounds.Right - 1.5f, cameraBounds.Left + 1.5f), cameraBounds.Top, 0.0f });
		glm::vec2 scale({ 1.0f, 1.0f });
		float rot = 180.0f;
		Enemy* enemy = ZeoEngine::Level::Get().SpawnGameObject<Enemy>(pos, scale, rot);
		if (enemy)
		{
			enemy->m_OnDestroyed = std::bind([&]() {
				DelaySpawnEnemy(2.0f);
			});
		}
	});
}

void GameManager::SpawnObstacle()
{
	// "Spawn" an obstacle from pool
	Obstacle* obstacle = m_ObstaclePool->GetNextPooledObject();
}
