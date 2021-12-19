#include "Editors/EditorBase.h"

#include "EditorUIRenderers/EditorUIRendererBase.h"
#include "Panels/OpenAssetPanel.h"
#include "Panels/SaveAssetPanel.h"
#include "Engine/Profile/Instrumentor.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	EditorBase::EditorBase(const char* editorName)
		: m_EditorName(editorName)
	{
	}

	EditorBase::~EditorBase() = default;

	void EditorBase::OnAttach()
	{
		m_EditorUIRenderer = CreateEditorUIRenderer();
		m_EditorUIRenderer->OnAttach();

		m_SceneRenderer = CreateSceneRenderer();
		m_SceneRenderer->OnAttach();
		m_PostSceneCreate.connect<&RenderSystem::UpdateScene>(m_SceneRenderer->GetRenderSystem());
		m_SceneRenderer->m_PostSceneRenderDel.connect<&EditorBase::PostSceneRender>(this);
		m_OnViewportResize.connect<&SceneRenderer::OnViewportResize>(m_SceneRenderer);

		// Create scene at last so that delegates bound beforehand will be called here
		NewScene();
	}

	void EditorBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnUpdate(dt);
		m_ActiveScene->OnUpdate(dt);
		m_SceneRenderer->OnRender();
	}

	void EditorBase::OnImGuiRender()
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnImGuiRender();
	}

	void EditorBase::OnEvent(Event& e)
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnEvent(e);

		if (!m_bBlockSceneEvents)
		{
			m_ActiveScene->OnEvent(e);
		}
	}

	void EditorBase::FocusContextEntity()
	{
		m_EditorCamera->StartFocusEntity(m_ContextEntity);
	}

	const Ref<FrameBuffer>& EditorBase::GetFrameBuffer() const
	{
		return m_SceneRenderer->GetFrameBuffer();
	}

	void EditorBase::Open()
	{
		m_bShow = true;
	}

	void EditorBase::NewScene(bool bIsFromLoad)
	{
		m_PreSceneCreateDel.publish(bIsFromLoad);
		m_ActiveScene = CreateScene();
		m_ActiveScene->OnAttach();
		m_PostSceneCreateDel.publish(m_ActiveScene, bIsFromLoad);
	}

	void EditorBase::LoadScene()
	{
		auto openAssetPanel = m_EditorUIRenderer->GetPanel<OpenAssetPanel>(OPEN_ASSET);
		if (!openAssetPanel)
		{
			openAssetPanel = m_EditorUIRenderer->CreatePanel<OpenAssetPanel>(OPEN_ASSET, GetAssetTypeId());
		}
		else
		{
			openAssetPanel->Open();
		}
	}

	void EditorBase::LoadScene(const std::string& path)
	{
		BenchmarkTimer timer;

		NewScene(true);
		LoadAsset(path);
		m_ActiveScene->PostLoad();
		m_PostSceneLoadDel.publish();

		ZE_CORE_WARN("Loading \"{0}\" took {1} ms", path, timer.ElapsedMillis());
	}

	void EditorBase::SaveScene()
	{
		const std::string assetPath = GetAsset()->GetPath();
		if (assetPath.empty())
		{
			SaveSceneAs();
		}
		else
		{
			SaveScene(assetPath);
		}
	}

	void EditorBase::SaveScene(const std::string& path)
	{
		BenchmarkTimer timer;

		SaveAsset(path);

		ZE_CORE_WARN("Saving {0} took {1} ms", path, timer.ElapsedMillis());
	}

	void EditorBase::SaveSceneAs()
	{
		auto saveAssetPanel = m_EditorUIRenderer->GetPanel<SaveAssetPanel>(SAVE_ASSET);
		if (!saveAssetPanel)
		{
			saveAssetPanel = m_EditorUIRenderer->CreatePanel<SaveAssetPanel>(SAVE_ASSET, GetAssetTypeId());
		}
		else
		{
			saveAssetPanel->Open();
		}
	}

	void EditorBase::PostSceneRender(const Ref<FrameBuffer>& fbo)
	{
		m_PostSceneRenderDel.publish(fbo);
	}

}
