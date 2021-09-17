#include "Scenes/EditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"
#include "Core/EditorManager.h"
#include "Core/EditorTypes.h"
#include "Editors/SceneEditor.h"

namespace ZeoEngine {

	EditorScene::EditorScene(const Ref<SceneEditor>& sceneEditor)
		: m_SceneEditor(sceneEditor)
	{
		m_RenderSystem = CreateRef<RenderSystem>(this);
		m_RenderSystem->OnCreate();
		m_NativeScriptSystem = CreateRef<NativeScriptSystem>(this);
		m_NativeScriptSystem->OnCreate();
	}

	EditorScene::~EditorScene()
	{
		m_RenderSystem->OnDestroy();
		m_NativeScriptSystem->OnDestroy();
	}

	void EditorScene::OnUpdate(DeltaTime dt)
	{
		switch (m_SceneEditor->GetSceneState())
		{
			case SceneState::Edit:
				OnUpdateEditor(dt);
				break;
			case SceneState::Play:
				OnUpdateRuntime(dt);
				break;
		}
	}

	void EditorScene::OnRender(const EditorCamera& camera)
	{
		switch (m_SceneEditor->GetSceneState())
		{
			case SceneState::Edit:
				OnRenderEditor(camera);
				break;
			case SceneState::Play:
			case SceneState::Pause:
				OnRenderRuntime();
				break;
		}
	}

	void EditorScene::OnEvent(Event& e)
	{
		m_NativeScriptSystem->OnEvent(e);
	}

	void EditorScene::OnUpdateEditor(DeltaTime dt)
	{
		m_RenderSystem->OnUpdate(dt);
	}

	void EditorScene::OnUpdateRuntime(DeltaTime dt)
	{
		m_NativeScriptSystem->OnUpdate(dt);
		m_RenderSystem->OnUpdate(dt);
	}

	void EditorScene::OnRenderEditor(const EditorCamera& camera)
	{
		m_RenderSystem->OnRenderEditor(camera);
	}

	void EditorScene::OnRenderRuntime()
	{
		m_RenderSystem->OnRenderRuntime();
	}

	void EditorScene::PostLoad()
	{
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& particleComp)
		{
			ParticleSystemInstance::Create(particleComp);
		});
	}

}
