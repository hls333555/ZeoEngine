#pragma once

namespace ZeoEngine {

	template<typename T, uint32_t C>
	class ObjectPooler
	{
	public:
		ObjectPooler(GameObject* owner = nullptr)
		{
			for (uint32_t i = 0; i < C; ++i)
			{
				T* object = Level::Get().SpawnGameObject<T>(owner);
				object->SetActive(false);
				m_Objects.push_back(object);
			}
		}

		const std::vector<T*>& GetPooledObjects() const { return m_Objects; }

		T* GetNextPooledObject() const
		{
			for (T* object : m_Objects)
			{
				if (!object->IsActive())
				{
					object->Reset();
					object->SetActive(true);
					return object;
				}
			}

			return nullptr;
		}

	private:
		std::vector<T*> m_Objects;
	};

}
