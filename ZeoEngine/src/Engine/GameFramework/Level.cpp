#include "ZEpch.h"
#include "Engine/GameFramework/Level.h"

#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/EngineGlobals.h"
#include "Engine/Core/Application.h"

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

	}

	void Level::OnUpdate(DeltaTime dt)
	{
		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (m_GameObjects[i]->IsPendingDestroy() || !m_GameObjects[i]->IsActive())
				continue;

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

	void Level::OnEditorUpdate(DeltaTime dt)
	{

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
		game->AddGameObjectPendingDestroy(object);

		// NOTE: DO NOT USE index based for loop here as it will not iterate all elements!
		for (auto it = m_GameObjects.begin(); it != m_GameObjects.end();)
		{
			// Remove this GameObject
			if (*it == object)
			{
				it = m_GameObjects.erase(it);
			}
			// Remove GameObjects that depend on this one
			else if ((*it)->GetOwner() == object)
			{
				(*it)->m_bPendingDestroy = true;
				(*it)->m_bIsActive = false;
				it = m_GameObjects.erase(it);
			}
			else
			{
				++it;
			}
		}

		auto it2 = std::find_if(m_TranslucentObjects.begin(), m_TranslucentObjects.end(), [&object](const std::pair<TranslucentObjectData, GameObject*>& objectPair) {
			return objectPair.second == object;
		});
		if (it2 != m_TranslucentObjects.end())
		{
			m_TranslucentObjects.erase(it2);
		}
	}

	ParticleSystem* Level::SpawnParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent, bool bAutoDestroy)
	{
		ParticleSystem* ps = new ParticleSystem(particleTemplate, attachToParent, bAutoDestroy);
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

}
