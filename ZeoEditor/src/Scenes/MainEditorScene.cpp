#include "Scenes/MainEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	MainEditorScene::MainEditorScene()
	{
		m_RenderSystem = CreateRef<RenderSystem>(this);
		m_RenderSystem->OnCreate();
		m_NativeScriptSystem = CreateRef<NativeScriptSystem>(this);
		m_NativeScriptSystem->OnCreate();
	}

	MainEditorScene::~MainEditorScene()
	{
		m_RenderSystem->OnDestroy();
		m_NativeScriptSystem->OnDestroy();
	}

	void MainEditorScene::OnUpdate(DeltaTime dt)
	{
		// TODO:
		OnUpdateEditor(dt);
	}

	void MainEditorScene::OnRender(const EditorCamera& camera)
	{
		// TODO:
		OnRenderEditor(camera);
	}

	void MainEditorScene::OnEvent(Event& e)
	{
		m_NativeScriptSystem->OnEvent(e);
	}

	void MainEditorScene::OnUpdateEditor(DeltaTime dt)
	{
		m_RenderSystem->OnUpdate(dt);
	}

	void MainEditorScene::OnUpdateRuntime(DeltaTime dt)
	{
		m_NativeScriptSystem->OnUpdate(dt);
		m_RenderSystem->OnUpdate(dt);
	}

	void MainEditorScene::OnRenderEditor(const EditorCamera& camera)
	{
		m_RenderSystem->OnRenderEditor(camera);
	}

	void MainEditorScene::OnRenderRuntime()
	{
		m_RenderSystem->OnRenderRuntime();
	}

	void MainEditorScene::PostLoad()
	{
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& particleComp)
		{
			ParticleSystemInstance::Create(particleComp);
		});
	}

}
