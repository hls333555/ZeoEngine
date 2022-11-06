#include "ZEpch.h"
#include "Engine/GameFramework/World.h"

#include "Engine/Profile/Profiler.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	WorldBase::WorldBase(std::string worldName)
		: m_WorldName(std::move(worldName))
	{
	}

	WorldBase::~WorldBase() = default;

	void WorldBase::OnAttach()
	{
		NewScene();
		NewSceneRenderer();
	}

	void WorldBase::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		if (!m_bActive) return;

		if (IsRuntime())
		{
			if (IsRunning())
			{
				for (const auto& system : m_Systems)
				{
					system->OnUpdateRuntime(dt);
				}
			}
		}
		else
		{
			for (const auto& system : m_Systems)
			{
				system->OnUpdateEditor(dt);
			}
		}

		m_ActiveScene->OnUpdate();

		if (m_SceneRenderer)
		{
			m_SceneRenderer->OnRender();
		}
	}

	void WorldBase::NewScene()
	{
		const Ref<Scene> scene = CreateRef<Scene>(CreateSceneObserverSystem());
		ScriptEngine::SetSceneContext(scene);
		SetActiveScene(scene);
		PostSceneCreate(scene); // TODO:
	}

	void WorldBase::SetActiveScene(Ref<Scene> scene)
	{
		if (scene == m_ActiveScene) return;

		const auto lastScene = m_ActiveScene;
		m_ActiveScene = std::move(scene);
		m_OnActiveSceneChangedDel.publish(m_ActiveScene.get(), lastScene.get());
	}

	void WorldBase::NewSceneRenderer()
	{
		m_SceneRenderer = CreateSceneRenderer();
		if (m_SceneRenderer)
		{
			m_SceneRenderer->OnAttach(this);
		}
	}

}
