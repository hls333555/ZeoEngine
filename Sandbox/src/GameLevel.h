#pragma once

#include "ZeoEngine.h"

#include "Engine/GameFramework/ObjectPooler.h"

#include "Obstacle.h"

class Player;

class GameLevel : public ZeoEngine::Level
{
public:
	virtual void Init() override;
	virtual void OnUpdate(ZeoEngine::DeltaTime dt);

	inline Player* GetPlayer() { return m_Player; }

	void DelaySpawnEnemy(float delay);

private:
	void SpawnObstacle();

private:
	Player* m_Player;

	typedef ZeoEngine::ObjectPooler<Obstacle, 10> ObstaclePool;
	ZeoEngine::Scope<ObstaclePool> m_ObstaclePool;

	bool m_bShouldSpawnObstacle = true;

};
