#pragma once

#include "ZeoEngine.h"

#include "GameObject.h"
#include "ObjectPooler.h"
#include "Obstacle.h"

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

	const LevelBounds& GetLevelBounds() const { return m_LevelBounds; }
	void SetLevelBounds(const LevelBounds& levelBounds) { m_LevelBounds = levelBounds; }

	template<typename T>
	T* SpawnGameObject()
	{
		T* object = new T();
		object->SetName(ConstructObjectName<T>());
		m_GameObjects.push_back(object);
		object->Init(); // Should be put after pushing back to m_GameObjects!
		if (object->IsTranslucent())
		{
			m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
		}
		return object;
	}
	template<typename T>
	T* SpawnGameObject(const Transform& transform)
	{
		T* object = new T();
		object->SetName(ConstructObjectName<T>());
		m_GameObjects.push_back(object);
		object->Init(); // Should be put after pushing back to m_GameObjects!
		object->SetTransform(transform);
		if (object->IsTranslucent())
		{
			m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object; // Should be put after updating position!
		}
		return object;
	}
	template<typename T>
	T* SpawnGameObject(const glm::vec3& position, const glm::vec2& scale = { 1.0f, 1.0f }, float rotation = 0.0f)
	{
		T* object = new T();
		object->SetName(ConstructObjectName<T>());
		m_GameObjects.push_back(object);
		object->Init(); // Should be put after pushing back to m_GameObjects!
		object->SetPosition(position);
		object->SetRotation(rotation);
		object->SetScale(scale);
		if (object->IsTranslucent())
		{
			m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object; // Should be put after updating position!
		}
		return object;
	}

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

	void SpawnObstacles();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_backgroundTexture;

	LevelBounds m_LevelBounds;

	std::vector<GameObject*> m_GameObjects;
	std::unordered_map<std::string, uint32_t> m_ObjectNames;
	std::map<TranslucentObjectData, GameObject*> m_TranslucentObjects;
	uint32_t m_TranslucentObjectIndex = 0;

	typedef ObjectPooler<Obstacle, 10> ObstaclePool;
	ZeoEngine::Scope<ObstaclePool> m_ObstaclePool;

	bool m_bShouldSpawnObstacle = true;
};
