#pragma once

#include "Engine/GameFramework/GameObject.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/Core/Delegate.h"

namespace ZeoEngine {

	struct TranslucentObjectData
	{
		bool operator<(const TranslucentObjectData& other) const
		{
			// Sort based on Z position or based on spawn order if Z position equals
			if (abs(ZPosition - other.ZPosition) < 1e-8)
			{
				return Index < other.Index;
			}
			return ZPosition < other.ZPosition;
		}

		float ZPosition;
		uint32_t Index;
	};

	constexpr const char* LevelFileToken = "Level";

	class Level
	{
		friend class EngineLayer;
		friend class EditorLayer;
		friend class GameLayer;
		friend class GameObject;

		using LevelCleanUpDel = Delegate<void()>;

	public:
		~Level();
		Level(const Level&) = delete;
		Level& operator=(const Level&) = delete;

		static Level& Get()
		{
			static Level level;
			return level;
		}

	private:
		Level() = default;

	public:
		virtual void Init();

		virtual void OnUpdate(DeltaTime dt);
		void OnRender();
		virtual void OnImGuiRender();

		TimerManager* GetGameTimerManager() { return &m_GameTimerManager; }

		template<typename T>
		T* SpawnGameObject(GameObject* owner = nullptr)
		{
			T* object = PreConstructObject<T>(owner);
			PostConstructObject(object);
			return object;
		}
		template<typename T>
		T* SpawnGameObject(const Transform& transform, GameObject* owner = nullptr)
		{
			T* object = PreConstructObject<T>(owner);
			object->SetTransform(transform);
			PostConstructObject(object);
			return object;
		}
		template<typename T>
		T* SpawnGameObject(const glm::vec3& position, const glm::vec2& scale = { 1.0f, 1.0f }, float rotation = 0.0f, GameObject* owner = nullptr)
		{
			T* object = PreConstructObject<T>(owner);
			object->SetPosition(position);
			object->SetRotation(rotation);
			object->SetScale(scale);
			PostConstructObject(object);
			return object;
		}

		// TODO: Need a way to destroy a particle system explicitly
		ParticleSystem* SpawnParticleSystemAtPosition(ParticleSystem* psTemplate, const glm::vec2& position, bool bAutoDestroy = true);
		ParticleSystem* SpawnParticleSystemAttached(ParticleSystem* psTemplate, GameObject* attachToParent, bool bAutoDestroy = true);

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
		template<typename T>
		T* PreConstructObject(GameObject* owner)
		{
			T* object = new T();
			ConstructObjectName<T>(object);
			object->SetOwner(owner);
			if (owner)
			{
				owner->AddOwningObjects(object);
			}
			return object;
		}
		template<typename T>
		void PostConstructObject(T* object)
		{
			object->Init();
			m_GameObjects.push_back(object);
			m_SortedGameObjects.emplace(std::make_pair(object->GetName(), object));
			if (object->IsTranslucent())
			{
				m_TranslucentObjects[{ object->GetPosition().z, m_TranslucentObjectIndex++ }] = object;
			}
		}

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
				m_ObjectNames.emplace(ss2.str());
				object->SetName(ss2.str());
			}
		}

		void PendingDestroyGameObject(GameObject* object);
		/** Remove GameObject pointers from containers but do not delete them. Called when garbage collection starts. */
		void RemoveGameObjects();

		/** Re-sort translucent objects when GameObject's position value is changed in Object Inspector window.  */
		void OnTranslucentObjectsDirty(GameObject* dirtyGameObject);

		void CleanUp();

		void LoadLevelFromFile(const char* levelPath, bool bIsTemp = false);
		void SaveLevelToFile(std::string& levelPath, bool bIsTemp = false);

		void PreDeserialize(const std::string& src);

	public:
		LevelCleanUpDel m_OnLevelCleanUp;

	private:
		Ref<Texture2D> m_backgroundTexture;

		/** Stores all spawned GameObjects in spawn order */
		std::vector<GameObject*> m_GameObjects;
		/** Map from GameObject's name to GameObject, used dedicated by Level Outline window */
		std::multimap<std::string, GameObject*> m_SortedGameObjects;
		std::unordered_map<std::string, uint32_t> m_ObjectUniqueNames;
		/** Stores all used object names */
		std::set<std::string> m_ObjectNames;
		std::map<TranslucentObjectData, GameObject*> m_TranslucentObjects;
		uint32_t m_TranslucentObjectIndex = 0;

		TimerManager m_GameTimerManager{ "Game" };
		ParticleManager m_ParticleManager;

		EngineLayer* m_EngineLayer;

	};

}
