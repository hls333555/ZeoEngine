#pragma once

#include "Engine/GameFramework/GameObject.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Core/Application.h"
#include "Engine/Layers/EditorLayer.h"

namespace ZeoEngine {

	struct TranslucentObjectData
	{
		bool operator<(const TranslucentObjectData& other) const
		{
			// Sort based on Z position or based on spawn order if Z position equals
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
		friend class EditorLayer;
		friend void GameObject::Destroy();

	private:
		Level() = default;

		Level(const Level&) = delete;
		Level& operator=(const Level&) = delete;

	public:
		virtual ~Level();

		static Level& Get()
		{
			static Level level;
			return level;
		}

		virtual void Init();

		virtual void OnUpdate(DeltaTime dt);
		void OnEditorUpdate(DeltaTime dt);
		void OnRender();
		virtual void OnImGuiRender();

		template<typename T>
		T* SpawnGameObject(GameObject* owner = nullptr)
		{
			T* object = new T();
			ConstructObjectName<T>(object);
			object->SetOwner(owner);
			object->Init();
			m_GameObjects.push_back(object);
			m_SortedGameObjects.insert({ object->GetName(), object });
			if (object->IsTranslucent())
			{
				m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
			}
			return object;
		}
		template<typename T>
		T* SpawnGameObject(const Transform& transform, GameObject* owner = nullptr)
		{
			T* object = new T();
			ConstructObjectName<T>(object);
			object->SetOwner(owner);
			object->SetTransform(transform);
			object->Init();
			m_GameObjects.push_back(object);
			m_SortedGameObjects.insert({ object->GetName(), object });
			if (object->IsTranslucent())
			{
				m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
			}
			return object;
		}
		template<typename T>
		T* SpawnGameObject(const glm::vec3& position, const glm::vec2& scale = { 1.0f, 1.0f }, float rotation = 0.0f, GameObject* owner = nullptr)
		{
			T* object = new T();
			ConstructObjectName<T>(object);
			object->SetOwner(owner);
			object->SetPosition(position);
			object->SetRotation(rotation);
			object->SetScale(scale);
			object->Init();
			m_GameObjects.push_back(object);
			m_SortedGameObjects.insert({ object->GetName(), object });
			if (object->IsTranslucent())
			{
				m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
			}
			return object;
		}

		// TODO: Need a way to destroy a particle system explicitly
		ParticleSystem* SpawnParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true);

		template<typename T>
		std::vector<GameObject*> GetAllObjectsOfClass()
		{
			std::vector<GameObject*> result;
			for (auto* object : m_GameObjects)
			{
				if (dynamic_cast<T*>(object))
				{
					result.push_back(object);
				}
			}
			return result;
		}

	private:
		// TODO: Should strip out "ZeoEngine::" in some cases
		template<typename T>
		void ConstructObjectName(GameObject* object)
		{
			std::string s(typeid(T).name());
			// TODO: Strip out "class " prefix, this behavior is compiler-dependent!
			s = s.substr(6);
			std::stringstream ss1, ss2;
			// Object's unique name
			{
				ss1 << s;
				uint32_t count = ++m_ObjectUniqueNames[s];
				if (count > 1)
				{
					ss1 << "_" << count;
				}
				object->SetUniqueName(ss1.str());
			}
			// Object's display name
			{
				ss2 << s;
				uint32_t count = 1;
				while (m_ObjectNames.find(ss2.str()) != m_ObjectNames.end())
				{
					ss2.clear();
					ss2.str("");
					ss2 << s << "_" << ++count;
				}
				m_ObjectNames.insert(ss2.str());
				object->SetName(ss2.str());
			}
		}

		void PendingDestroyGameObject(GameObject* object);
		void RemoveGameObject(GameObject* object);

		/** Re-sort translucent objects when GameObject's position value is changed in Object Property window.  */
		void OnTranslucentObjectsDirty(GameObject* dirtyGameObject);

		void CleanUp();

		void LoadLevelFromFile(const char* levelPath, bool bIsTemp = false);
		void SaveLevelToFile(std::string& levelPath, bool bIsTemp = false);

		void PreDeserialize(const std::string& src);

	private:
		Ref<Texture2D> m_backgroundTexture;

		/** Stores all created GameObjects */
		std::vector<GameObject*> m_GameObjects;
		/** Map from GameObject's name to GameObject, used dedicated by Level Outline window */
		std::multimap<std::string, GameObject*> m_SortedGameObjects;
		std::unordered_map<std::string, uint32_t> m_ObjectUniqueNames;
		/** Stores all used object names */
		std::set<std::string> m_ObjectNames;
		std::map<TranslucentObjectData, GameObject*> m_TranslucentObjects;
		uint32_t m_TranslucentObjectIndex = 0;

		ParticleManager m_ParticleManager;

	};

}
