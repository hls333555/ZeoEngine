#pragma once

namespace ZeoEngine {

	template<typename T, uint32_t C>
	class ObjectPooler
	{
	public:
		explicit ObjectPooler(GameObject* owner = nullptr);

		const std::vector<T*>& GetPooledObjects() const { return m_Objects; }
		uint32_t Size() const { return C; }

		T* GetNextPooledObject() const;

	private:
		std::vector<T*> m_Objects;
	};

	template<typename T, uint32_t C>
	ObjectPooler<T, C>::ObjectPooler(GameObject* owner)
	{
		for (uint32_t i = 0; i < C; ++i)
		{
			T* object = Level::Get().SpawnGameObject<T>(owner);
			object->SetActive(false);
			m_Objects.push_back(object);
		}
	}

	template<typename T, uint32_t C>
	T* ObjectPooler<T, C>::GetNextPooledObject() const
	{
		for (auto* object : m_Objects)
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

}
