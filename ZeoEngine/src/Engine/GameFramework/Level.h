#pragma once

#include "Engine/GameFramework/GameObject.h"
#include "Engine/GameFramework/ParticleSystem.h"

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
		Level()
		{
			ZE_CORE_TRACE("Creating level...");
		}

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
			std::string uniqueName = ConstructObjectName<T>();
			object->SetUniqueName(uniqueName);
			object->SetName(uniqueName);
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
			std::string uniqueName = ConstructObjectName<T>();
			object->SetUniqueName(uniqueName);
			object->SetName(uniqueName);
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
			std::string uniqueName = ConstructObjectName<T>();
			object->SetUniqueName(uniqueName);
			object->SetName(uniqueName);
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

		// TODO: Need a way to destroy a particle system explicitly
		ParticleSystem* SpawnParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true);

		/** Re-sort translucent objects when GameObject's position value is changed in Object Property window.  */
		void OnTranslucentObjectsDirty(GameObject* dirtyGameObject);

	private:
		// TODO: Should strip out "ZeoEngine::" in some cases
		template<typename T>
		std::string ConstructObjectName()
		{
			std::string s(typeid(T).name());
			// TODO: Strip out "class " prefix, this behavior is compiler-dependent!
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

		/** DO NOT call it directly, call GameObject::Destroy() instead! This is for internal use only! */
		void PendingDestroyGameObject(GameObject* object);

	private:
		Ref<Texture2D> m_backgroundTexture;

		std::vector<GameObject*> m_GameObjects;
		std::unordered_map<std::string, uint32_t> m_ObjectNames;
		std::map<TranslucentObjectData, GameObject*> m_TranslucentObjects;
		uint32_t m_TranslucentObjectIndex = 0;

		ParticleManager m_ParticleManager;

	};

}
