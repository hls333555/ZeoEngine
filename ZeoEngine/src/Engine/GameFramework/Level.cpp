#include "ZEpch.h"
#include "Engine/GameFramework/Level.h"

#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/EngineGlobals.h"
#include "Engine/Core/Application.h"
#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Level::~Level()
	{
		for (auto* object : m_GameObjects)
		{
			delete object;
		}
	}

	void Level::Init()
	{
		Texture2DLibrary* library = GetTexture2DLibrary();
		ZE_CORE_ASSERT(library);
		// Default level background
		m_backgroundTexture = library->Load("editor_assets/textures/Checkerboard_Alpha.png");

		// TODO: Move it to config file
		// Load default game level when out of editor
#if !WITH_EDITOR
		LoadLevelFromFile("assets/test.zlevel");
#endif

	}

	void Level::OnUpdate(DeltaTime dt)
	{
		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (m_GameObjects[i]->IsPendingDestroy() || !m_GameObjects[i]->IsActive())
				continue;

			if (!m_GameObjects[i]->HasBegunPlay())
			{
				m_GameObjects[i]->BeginPlay();
			}

			// Collision detection
			if (m_GameObjects[i]->IsCollisionEnabled() && m_GameObjects[i]->ShouldGenerateOverlapEvents())
			{
				m_GameObjects[i]->DoCollisionTest(m_GameObjects);
			}

			// Out of range error may occur
			if (i < m_GameObjects.size())
			{
				m_GameObjects[i]->OnUpdate(dt);
			}
		}

		m_ParticleManager.OnUpdate(dt);

	}

	void Level::OnRender()
	{
		// We assume zFar of orthographic projection is always -1, so we draw background texture as far as we can
		// TODO: Not applied to 3D rendering
		Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.99f }, { 100.0f, 100.0f }, m_backgroundTexture, { 50.0f, 50.0f });

		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (m_GameObjects[i]->IsPendingDestroy() || !m_GameObjects[i]->IsActive())
				continue;

			if (!m_GameObjects[i]->IsTranslucent())
			{
				// Render opaque objects first
				m_GameObjects[i]->OnRender();
			}
		}

		RenderCommand::EnableDepthWriting(false);
		// Then render translucent objects
		for (const auto& objectPair : m_TranslucentObjects)
		{
			if (!objectPair.second->IsActive())
				continue;

			objectPair.second->OnRender();
		}
		m_ParticleManager.OnRender();
		RenderCommand::EnableDepthWriting(true);

	}

	void Level::OnImGuiRender()
	{
		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (!m_GameObjects[i]->IsPendingDestroy() && m_GameObjects[i]->IsActive())
			{
				m_GameObjects[i]->OnImGuiRender();
			}
		}
	}

	void Level::PendingDestroyGameObject(GameObject* object)
	{
		if (!object)
			return;

		GameLayer* game = Application::Get().FindLayerByName<GameLayer>("Game");
		// NOTE: DO NOT USE index based for loop here as it will not iterate all elements!
		for (auto it = m_GameObjects.begin(); it != m_GameObjects.end();)
		{
			// Remove this GameObject
			if (*it == object)
			{
				game->AddGameObjectPendingDestroy(object);
				RemoveGameObject(object);
				it = m_GameObjects.erase(it);
			}
			// Remove GameObjects that depend on this one
			else if ((*it)->GetOwner() == object)
			{
				(*it)->m_bPendingDestroy = true;
				(*it)->m_bIsActive = false;
				game->AddGameObjectPendingDestroy(*it);
				RemoveGameObject(*it);
				it = m_GameObjects.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void Level::RemoveGameObject(GameObject* object)
	{
		m_ObjectNames.erase(object->GetName());
		auto it = std::find_if(m_SortedGameObjects.begin(), m_SortedGameObjects.end(), [&object](const std::pair<std::string, GameObject*>& objectPair) {
			return objectPair.second == object;
			});
		if (it != m_SortedGameObjects.end())
		{
			m_SortedGameObjects.erase(it);
		}
		auto it2 = std::find_if(m_TranslucentObjects.begin(), m_TranslucentObjects.end(), [&object](const std::pair<TranslucentObjectData, GameObject*>& objectPair) {
			return objectPair.second == object;
			});
		if (it2 != m_TranslucentObjects.end())
		{
			m_TranslucentObjects.erase(it2);
		}
	}

	ParticleSystem* Level::SpawnParticleSystemAtPosition(ParticleSystem* psTemplate, const glm::vec2& position, bool bAutoDestroy)
	{
		if (!psTemplate)
			return nullptr;

		ParticleSystem* ps = new ParticleSystem(psTemplate->GetParticleTemplate(), position, bAutoDestroy);
		m_ParticleManager.AddParticleSystem(ps);
		return ps;
	}

	ParticleSystem* Level::SpawnParticleSystemAttached(ParticleSystem* psTemplate, GameObject* attachToParent, bool bAutoDestroy)
	{
		if (!psTemplate)
			return nullptr;

		ParticleSystem* ps = new ParticleSystem(psTemplate->GetParticleTemplate(), attachToParent, bAutoDestroy);
		m_ParticleManager.AddParticleSystem(ps);
		return ps;
	}

	void Level::OnTranslucentObjectsDirty(GameObject* dirtyGameObject)
	{
		auto it = std::find_if(m_TranslucentObjects.begin(), m_TranslucentObjects.end(), [&dirtyGameObject](const std::pair<TranslucentObjectData, GameObject*>& objectPair) {
			return objectPair.second == dirtyGameObject;
		});
		if (it != m_TranslucentObjects.end())
		{
			uint32_t index = it->first.index;
			float zPos = dirtyGameObject->GetPosition().z;
			// Only process re-sorting if Z position is changed
			if (abs(it->first.zPosition - zPos) >= 1e-8)
			{
				m_TranslucentObjects.erase(it);
				m_TranslucentObjects[{ zPos, index }] = dirtyGameObject;
			}
		}
	}

	void Level::CleanUp()
	{
		BenchmarkTimer bt(false);

		for (auto* object : m_GameObjects)
		{
			delete object;
		}
		m_GameObjects.clear();
		m_SortedGameObjects.clear();
		m_ObjectUniqueNames.clear();
		m_ObjectNames.clear();
		m_TranslucentObjects.clear();
		m_TranslucentObjectIndex = 0;
#if WITH_EDITOR
		EditorLayer* editor = Application::Get().FindLayerByName<EditorLayer>("Editor");
		editor->ClearSelectedGameObject();
#endif

		m_ParticleManager.CleanUp();

		ZE_CORE_INFO("Level cleanup took {0}s.", bt.GetDuration());
	}

	void Level::LoadLevelFromFile(const char* levelPath, bool bIsTemp)
	{
		std::string result;
		if (!Serializer::Get().ValidateFile(levelPath, LevelFileToken, result))
			return;

		CleanUp();

		if (!bIsTemp)
		{
			ZE_CORE_TRACE("Start loading level: {0}", levelPath);
		}
		BenchmarkTimer bt(false);
		PreDeserialize(result);
		if (!bIsTemp)
		{
			ZE_CORE_INFO("Level loading took {0}s.", bt.GetDuration());
		}
	}
	// TODO: Add version checking
	void Level::PreDeserialize(const std::string& src)
	{
		static const char* classNameLineStart = "#";
		static const char* classNameStart = "(";
		static const char* classNameEnd = ")";

		size_t classNameLineStartPos = 0;
		// Start of class name
		size_t classNameStartPos = src.find(classNameStart, 0);
		while (classNameStartPos != std::string::npos)
		{
			// End of class name
			size_t classNameEndPos = src.find(classNameEnd, classNameStartPos);
			++classNameStartPos;
			// Get class name
			std::string className = src.substr(classNameStartPos, classNameEndPos - classNameStartPos);
			// Beginning of the next line
			size_t nextLinePos = src.find_first_of("\r\n", classNameEndPos);
			// Locate the next line of class name
			classNameLineStartPos = src.find(classNameLineStart, nextLinePos);
			classNameStartPos = src.find(classNameStart, classNameLineStartPos);
			std::string extractedObjectJson = src.substr(nextLinePos, classNameLineStartPos - nextLinePos);

			Serializer::Get().Deserialize<GameObject*>(extractedObjectJson, [&className]() {
				return rttr::type::get_by_name(className).create({ glm::vec3{ 0.0f, 0.0f, 0.1f } });
			});
		}
	}

	void Level::SaveLevelToFile(std::string& levelPath, bool bIsTemp)
	{
		static const char* levelFileSuffix = ".zlevel";
		if (!bIsTemp && levelPath.rfind(levelFileSuffix) == std::string::npos)
		{
			levelPath += levelFileSuffix;
		}
		std::ofstream out(levelPath, std::ios::out | std::ios::binary);
		if (!out)
		{
			ZE_CORE_ERROR("Could not open level file!");
			return;
		}
		if (!bIsTemp)
		{
			ZE_CORE_TRACE("Start saving level: {0}", levelPath);
		}
		BenchmarkTimer bt(false);
		out << "#type " << LevelFileToken << std::endl;
		for (auto* object : m_GameObjects)
		{
			// e.g. #Player_2(Player)
			out << "#" << object->GetUniqueName() << "(" << rttr::type::get(*object).get_name() << ")" << std::endl;
			// TODO: Save camera position
			out << Serializer::Get().Serialize(object) << std::endl;
		}
		if (!bIsTemp)
		{
			ZE_CORE_INFO("Level saving took {0}s.", bt.GetDuration());
		}
	}

}
