#pragma once

#include "ZeoEngine.h"

#include "GameObject.h"
#include "ObjectPooler.h"
#include "Obstacle.h"
#include "ParticleSystem.h"

class Player;

struct LevelBounds
{
	float left, right, bottom, top;
};

struct TranslucentObjectData
{
	bool operator<(const TranslucentObjectData& other) const
	{
		if (abs(zPosition - other.zPosition) < 1e-8)
		{
			return index < other.index;
		}
		return zPosition < other.zPosition;
	}

	float zPosition;
	uint32_t index;
};

class Level
{
	friend void GameObject::Destroy();

public:
	~Level();

	void Init();

	void OnUpdate(ZeoEngine::DeltaTime dt);
	void OnRender();
	void OnImGuiRender();

	inline const LevelBounds& GetLevelBounds() const { return m_LevelBounds; }
	inline void SetLevelBounds(const LevelBounds& levelBounds) { m_LevelBounds = levelBounds; }
	inline Player* GetPlayer() { return m_Player; }

	template<typename T>
	T* SpawnGameObject(GameObject* owner = nullptr)
	{
		T* object = new T();
		object->SetName(ConstructObjectName<T>());
		object->SetOwner(owner);
		object->Init();
		m_GameObjects.push_back(object);
		if (object->IsTranslucent())
		{
			m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
		}
		object->BeginPlay();
		return object;
	}
	template<typename T>
	T* SpawnGameObject(const Transform& transform, GameObject* owner = nullptr)
	{
		T* object = new T();
		object->SetName(ConstructObjectName<T>());
		object->SetOwner(owner);
		object->SetTransform(transform);
		object->Init();
		m_GameObjects.push_back(object);
		if (object->IsTranslucent())
		{
			m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
		}
		object->BeginPlay();
		return object;
	}
	template<typename T>
	T* SpawnGameObject(const glm::vec3& position, const glm::vec2& scale = { 1.0f, 1.0f }, float rotation = 0.0f, GameObject* owner = nullptr)
	{
		T* object = new T();
		object->SetName(ConstructObjectName<T>());
		object->SetOwner(owner);
		object->SetPosition(position);
		object->SetRotation(rotation);
		object->SetScale(scale);
		object->Init();
		m_GameObjects.push_back(object);
		if (object->IsTranslucent())
		{
			m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
		}
		object->BeginPlay();
		return object;
	}

	ParticleSystem* SpawnParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true);

	void DelaySpawnEnemy(float delay);

private:
	template<typename T>
	std::string ConstructObjectName()
	{
		std::string s(typeid(T).name());
		// Strip out "class " prefix
		s = s.substr(6);
		std::stringstream ss;
		uint32_t count = ++m_ObjectNames[s];
		if (count == 1)
		{
			ss << s;
		}
		else
		{
			ss << s << "_" << count;
		}
		return ss.str();
	}

	void DestroyGameObject(GameObject* object);

	void SpawnObstacle();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_backgroundTexture;

	LevelBounds m_LevelBounds;

	std::vector<GameObject*> m_GameObjects;
	std::unordered_map<std::string, uint32_t> m_ObjectNames;
	std::map<TranslucentObjectData, GameObject*> m_TranslucentObjects;
	uint32_t m_TranslucentObjectIndex = 0;

	ParticleManager m_ParticleManager;

	Player* m_Player;

	typedef ObjectPooler<Obstacle, 10> ObstaclePool;
	ZeoEngine::Scope<ObstaclePool> m_ObstaclePool;

	bool m_bShouldSpawnObstacle = true;

};
