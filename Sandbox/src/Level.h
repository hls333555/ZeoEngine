#pragma once

#include "ZeoEngine.h"

#include "Player.h"

class GameObject;

struct LevelBounds
{
	float left, right, bottom, top;
};

class Level
{
	friend void GameObject::Destroy();

public:
	Level() = default;
	~Level();

	void Init();

	void OnUpdate(ZeoEngine::DeltaTime dt) ;
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
		object->Init(); // Should be put at last!
		return object;
	}
	template<typename T>
	T* SpawnGameObject(const Transform& transform)
	{
		T* object = SpawnGameObject<T>();
		object->SetTransform(transform);
		return object;
	}
	template<typename T>
	T* SpawnGameObject(const glm::vec2& position, const glm::vec2& scale = { 1.0f, 1.0f }, const glm::vec2& rotation = { 0.0f, 0.0f })
	{
		T* object = SpawnGameObject<T>();
		object->SetPosition(position);
		object->SetRotation(rotation);
		object->SetScale(scale);
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
		uint32_t count = ++m_gameObjectNames[s];
		if (count == 1)
		{
			ss << s;
		}
		else
		{
			ss << s << count;
		}
		return ss.str();
	}

	void DestroyGameObject(GameObject* object);

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_backgroundTexture;

	LevelBounds m_LevelBounds;

	std::vector<GameObject*> m_GameObjects;
	std::unordered_map<std::string, uint32_t> m_gameObjectNames;

};
