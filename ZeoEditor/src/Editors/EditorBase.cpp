#include "Editors/EditorBase.h"

#include "EditorUIRenderers/EditorUIRendererBase.h"
#include "Panels/OpenAssetPanel.h"
#include "Panels/SaveAssetPanel.h"
#include "Engine/Profile/Instrumentor.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderGraph.h"

namespace ZeoEngine {

	EditorBase::EditorBase(const char* editorName)
		: m_EditorName(editorName)
	{
	}

	EditorBase::~EditorBase() = default;

	void EditorBase::OnAttach()
	{
		m_FBO = CreateFrameBuffer();
		ZE_CORE_ASSERT(m_FBO);
		m_RenderGraph = CreateRenderGraph(m_FBO);
		m_EditorUIRenderer = CreateEditorUIRenderer();
		m_EditorUIRenderer->OnAttach();
		NewScene(); // Create scene at last so that delegates bound beforehand will be called here
	}

	void EditorBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		Renderer::SetActiveRenderGraph(m_RenderGraph.get());
		m_EditorUIRenderer->OnUpdate(dt);
		m_ActiveScene->OnUpdate(dt);
		m_FBO->Bind();
		{
			m_ActiveScene->OnRender(*m_EditorCamera);
			m_PostSceneRenderDel.publish(m_FBO);
			Renderer::FlushDebugDraws();
		}
		m_FBO->Unbind();
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

	void EditorBase::Open()
	{
		m_bShow = true;
	}

	void EditorBase::NewScene(bool bIsFromLoad)
	{
		m_PreSceneCreateDel.publish(bIsFromLoad);
		m_ActiveScene = CreateScene();
		m_PostSceneCreateDel.publish(bIsFromLoad);
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

}
