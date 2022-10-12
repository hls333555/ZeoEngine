#include "ZEpch.h"
#include "Engine/GameFramework/World.h"

#include "Engine/GameFramework/Scene.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	WorldBase::WorldBase(std::string worldName)
		: m_WorldName(std::move(worldName))
	{
	}

	void WorldBase::OnAttach()
	{
		NewScene();
		NewSceneRenderer();
	}

	void WorldBase::OnUpdate(DeltaTime dt)
	{
		m_ActiveScene->OnUpdate(dt);
		if (m_SceneRenderer)
		{
			m_SceneRenderer->OnRender();
		}
	}

	void WorldBase::NewScene()
	{
		Ref<Scene> scene = CreateScene();
		ScriptEngine::SetSceneContext(scene);
		scene->OnAttach(shared_from_this());
		PostSceneCreate(scene);
		SetActiveScene(std::move(scene));
	}

	void WorldBase::SetActiveScene(Ref<Scene> scene)
	{
		if (scene == m_ActiveScene) return;

		const auto lastScene = m_ActiveScene;
		m_ActiveScene = std::move(scene);
		m_OnActiveSceneChangedDel.publish(m_ActiveScene, lastScene);
	}

	void WorldBase::NewSceneRenderer()
	{
		m_SceneRenderer = CreateSceneRenderer();
		if (m_SceneRenderer)
		{
			m_SceneRenderer->OnAttach(shared_from_this());
		}
	}

}
