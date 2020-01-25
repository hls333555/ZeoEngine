#include "Level.h"

#include <imgui/imgui.h>

#include "Player.h"
#include "RandomEngine.h"
#include "ShooterGame.h"
#include "Enemy.h"

Level::~Level()
{
	for (auto* object : m_GameObjects)
	{
		delete object;
	}
}

void Level::Init()
{
	m_backgroundTexture = ZeoEngine::Texture2D::Create("assets/textures/Checkerboard_Alpha.png");

	m_ObstaclePool = ZeoEngine::CreateScope<ObstaclePool>(this);

	// Spawn player ship
	SpawnGameObject<Player>({ 0.0f, m_LevelBounds.bottom + 1.0f, 0.1f });

	// Spawn enemy ship
	DelaySpawnEnemy(5.0f);

}

void Level::OnUpdate(ZeoEngine::DeltaTime dt)
{
	for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
	{
		if (m_GameObjects[i]->IsActive())
		{
			// Collision detection
			if (m_GameObjects[i]->IsCollisionEnabled() && m_GameObjects[i]->ShouldGenerateOverlapEvent())
			{
				m_GameObjects[i]->DoCollisionTest(m_GameObjects);
			}

			// Out of range error may occur
			if (i < m_GameObjects.size())
			{
				m_GameObjects[i]->OnUpdate(dt);
			}
		}
	}

	// Spawn an obstacle every random intervals
	if (m_bShouldSpawnObstacle)
	{
		m_bShouldSpawnObstacle = false;
		float obstacleSpawnRate = RandomEngine::RandFloatInRange(1.0f, 4.0f);
		GetTimerManager()->SetTimer(obstacleSpawnRate, [&]() {
			m_bShouldSpawnObstacle = true;
		});
		
		SpawnObstacle();
	}

}

void Level::OnRender()
{
	ZeoEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 100.0f, 100.0f }, m_backgroundTexture, 50.0f);

	for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
	{
		if (!m_GameObjects[i]->IsTranslucent() && m_GameObjects[i]->IsActive())
		{
			// Render opaque objects first
			m_GameObjects[i]->OnRender();
		}
	}

	ZeoEngine::RenderCommand::EnableDepthWriting(false);
	// Then render translucent objects
	// NOTE: m_TranslucentObjects is not updated every frame
	// so you must make sure all translucent object's zPositions will never change at runtime!
	for (const auto& objectPair : m_TranslucentObjects)
	{
		if (objectPair.second->IsActive())
		{
			objectPair.second->OnRender();
		}
	}
	ZeoEngine::RenderCommand::EnableDepthWriting(true);
}

void Level::OnImGuiRender()
{
	for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
	{
		if (m_GameObjects[i]->IsActive())
		{
			m_GameObjects[i]->OnImGuiRender();
		}
	}

	// TODO: This will eventually be in EditorLayer
	ImGui::Begin("Level Outline");

	ImGui::Text("(%d objects total)", m_GameObjects.size());
	for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
	{
		ImVec4 color;
		if (m_GameObjects[i]->IsActive())
		{
			// Translucent objects are marked yellow instead of white
			color = m_GameObjects[i]->IsTranslucent() ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else
		{
			// Inactive objects are marked darker
			color = m_GameObjects[i]->IsTranslucent() ? ImVec4(0.75f, 0.75f, 0.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
		}
		ImGui::TextColored(color, "%s", m_GameObjects[i]->GetName().c_str());
	}

	ImGui::End();

	ImGui::Begin("Debug");

	ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);

	ImGui::End();
	
}

void Level::DestroyGameObject(GameObject* object)
{
	auto it = std::find(m_GameObjects.begin(), m_GameObjects.end(), object);
	if (it != m_GameObjects.end())
	{
		m_GameObjects.erase(it);
	}

	auto it2 = std::find_if(m_TranslucentObjects.begin(), m_TranslucentObjects.end(), [&object](const std::pair<TranslucentObjectData, GameObject*>& objectPair) {
		return objectPair.second == object;
	});
	if (it2 != m_TranslucentObjects.end())
	{
		m_TranslucentObjects.erase(it2);
	}
	
	delete object;
}

void Level::DelaySpawnEnemy(float delay)
{
	GetTimerManager()->SetTimer(delay, [&]() {
		glm::vec3 pos({ RandomEngine::RandFloatInRange(m_LevelBounds.right - 1.5f, m_LevelBounds.left + 1.5f), m_LevelBounds.top, 0.0f });
		glm::vec2 scale({ 1.0f, 1.0f });
		float rot = 180.0f;
		SpawnGameObject<Enemy>(pos, scale, rot);
	});
}

void Level::SpawnObstacle()
{
	// "Spawn" an obstacle from pool
	Obstacle* obstacle = m_ObstaclePool->GetNextPooledObject();
}
