#include "Editors/EditorBase.h"

#include "EditorUIRenderers/EditorUIRendererBase.h"
#include "Panels/OpenAssetPanel.h"
#include "Panels/SaveAssetPanel.h"
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

		NewDefaultScene();
		LoadAndApplyDefaultAsset();
		m_SceneRenderer = CreateSceneRenderer();
		if (m_SceneRenderer)
		{
			m_SceneRenderer->OnAttach(m_ActiveScene);
		}

		m_OnActiveSceneChanged.connect<&EditorBase::OnActiveSceneChanged>(this);
		if (m_SceneRenderer)
		{
			m_SceneRenderer->m_PostSceneRenderDel.connect<&EditorBase::PostSceneRender>(this);
			m_OnViewportResize.connect<&SceneRenderer::OnViewportResize>(m_SceneRenderer);
		}
	}

	void EditorBase::OnUpdate(DeltaTime dt) const
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnUpdate(dt);
		m_ActiveScene->OnUpdate(dt);
		if (m_SceneRenderer)
		{
			m_SceneRenderer->OnRender();
		}
	}

	void EditorBase::OnImGuiRender() const
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnImGuiRender();
	}

	void EditorBase::OnEvent(Event& e) const
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnEvent(e);

		if (!m_bBlockSceneEvents)
		{
			m_ActiveScene->OnEvent(e);
		}
	}

	void EditorBase::FocusContextEntity() const
	{
		m_EditorCamera->StartFocusEntity(m_ContextEntity);
	}

	const Ref<FrameBuffer>& EditorBase::GetFrameBuffer() const
	{
		return m_SceneRenderer->GetFrameBuffer();
	}

	void EditorBase::SetActiveScene(const Ref<Scene>& newScene, bool bIsCreateDefault)
	{
		if (newScene == m_ActiveScene) return;

		m_ActiveScene = newScene;
		m_OnActiveSceneChangedDel.publish(m_ActiveScene, bIsCreateDefault);
	}

	void EditorBase::NewDefaultScene()
	{
		NewScene(true);
	}

	void EditorBase::NewScene(bool bIsCreateDefault)
	{
		m_PreSceneCreateDel.publish(bIsCreateDefault);
		const auto scene = CreateScene();
		scene->OnAttach();
		m_ContextEntity = CreatePreviewEntity(scene);
		m_PostSceneCreateDel.publish(scene, bIsCreateDefault);
		SetActiveScene(scene, bIsCreateDefault);
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
		Timer timer;

		LoadAsset(path);
		m_ActiveScene->PostLoad();
		m_PostSceneLoadDel.publish();

		ZE_CORE_WARN("Loading \"{0}\" took {1} ms", path, timer.ElapsedMillis());
	}

	void EditorBase::SaveScene()
	{
		const std::string& assetPath = GetAsset()->GetID();
		if (assetPath.empty() || GetAsset()->IsTemplate())
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
		Timer timer;

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

	void EditorBase::Open()
	{
		m_bShow = true;
	}

	void EditorBase::OnActiveSceneChanged(const Ref<Scene>& scene, bool bIsCreateDefault)
	{
		if (m_SceneRenderer)
		{
			m_SceneRenderer->GetRenderSystem()->UpdateScene(m_ActiveScene);
			m_SceneRenderer->UpdateSceneContext(m_ActiveScene);
		}
		if (bIsCreateDefault)
		{
			LoadAndApplyDefaultAsset();
		}
	}

	void EditorBase::PostSceneRender(const Ref<FrameBuffer>& fbo) const
	{
		m_PostSceneRenderDel.publish(fbo);
	}

	void EditorBase::SaveAsset(const std::string& path)
	{
		GetAsset()->Serialize(path);
	}
}
