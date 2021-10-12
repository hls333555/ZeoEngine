#include "Scenes/LevelEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"
#include "Core/EditorManager.h"
#include "Core/EditorTypes.h"
#include "Editors/LevelEditor.h"

namespace ZeoEngine {

	LevelEditorScene::LevelEditorScene(const Ref<LevelEditor>& sceneEditor)
		: m_SceneEditor(sceneEditor)
	{
		m_RenderSystem = CreateScope<RenderSystem>(this);
		m_RenderSystem->OnCreate();
		m_NativeScriptSystem = CreateScope<NativeScriptSystem>(this);
		m_NativeScriptSystem->OnCreate();
		m_PhysicsSystem = CreateScope<PhysicsSystem>(this);
		m_PhysicsSystem->OnCreate();
	}

	LevelEditorScene::~LevelEditorScene()
	{
		m_RenderSystem->OnDestroy();
		m_NativeScriptSystem->OnDestroy();
		m_PhysicsSystem->OnDestroy();
	}

	void LevelEditorScene::OnUpdate(DeltaTime dt)
	{
		switch (m_SceneEditor->GetSceneState())
		{
			case SceneState::Edit:	OnUpdateEditor(dt); break;
			case SceneState::Play:	OnUpdateRuntime(dt); break;
		}
	}

	void LevelEditorScene::OnRender(const EditorCamera& camera)
	{
		switch (m_SceneEditor->GetSceneState())
		{
			case SceneState::Edit:	OnRenderEditor(camera); break;
			case SceneState::Play:
			case SceneState::Pause:	OnRenderRuntime(); break;
		}
	}

	void LevelEditorScene::OnEvent(Event& e)
	{
		m_NativeScriptSystem->OnEvent(e);
	}

	void LevelEditorScene::OnRuntimeStart()
	{
		m_PhysicsSystem->OnRuntimeStart();
	}

	void LevelEditorScene::OnRuntimeStop()
	{
		m_PhysicsSystem->OnRuntimeStop();
	}

	void LevelEditorScene::OnUpdateEditor(DeltaTime dt)
	{
		m_RenderSystem->OnUpdate(dt);
	}

	void LevelEditorScene::OnUpdateRuntime(DeltaTime dt)
	{
		m_NativeScriptSystem->OnUpdate(dt);
		m_PhysicsSystem->OnUpdate(dt);
		m_RenderSystem->OnUpdate(dt);
	}

	void LevelEditorScene::OnRenderEditor(const EditorCamera& camera)
	{
		m_RenderSystem->OnRenderEditor(camera);
	}

	void LevelEditorScene::OnRenderRuntime()
	{
		m_RenderSystem->OnRenderRuntime();
	}

	void LevelEditorScene::PostLoad()
	{
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& particleComp)
		{
			ParticleSystemInstance::Create(particleComp);
		});
	}

}
