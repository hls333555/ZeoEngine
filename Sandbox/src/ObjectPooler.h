#pragma once

#include "ZeoEngine.h"

extern class Level* GetLevel();

template<typename T, uint32_t C>
class ObjectPooler
{
public:
	ObjectPooler()
	{
		Level* level = GetLevel();
		if (level)
		{
			for (uint32_t i = 0; i < C; ++i)
			{
				T* object = level->SpawnGameObject<T>();
				object->SetActive(false);
				m_Objects.push_back(object);
			}
		}
	}

	inline const std::vector<T*>& GetPooledObjects() const { return m_Objects; }

	T* GetNextPooledObject() const
	{
		for (T* object : m_Objects)
		{
			if (!object->IsActive())
			{
				return object;
			}
		}

		return nullptr;
	}

private:
	std::vector<T*> m_Objects;
};
