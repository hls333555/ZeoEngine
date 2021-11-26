#include "Editors/EditorBase.h"

#include "EditorUIRenderers/EditorUIRendererBase.h"
#include "Panels/OpenAssetPanel.h"
#include "Panels/SaveAssetPanel.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Debug/Instrumentor.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Debug/BenchmarkTimer.h"

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

namespace ZeoEngine {

	EditorBase::EditorBase(const char* editorName)
		: m_EditorName(editorName)
	{
	}

	void EditorBase::OnAttach()
	{
		CreateFrameBuffer();
		m_EditorUIRenderer = CreateEditorUIRenderer();
		m_EditorUIRenderer->OnAttach();
		NewScene(); // Create scene at last so that delegates bound beforehand will be called here
	}

	void EditorBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnUpdate(dt);

		m_ActiveScene->OnUpdate(dt);

		BeginFrameBuffer();
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				glm::vec4 clearColor = { 0.2f, 0.2f, 0.2f, 1.0f };
				RenderCommand::SetClearColor(clearColor);
				RenderCommand::Clear();
				// Clear entity ID buffer to -1
				glm::vec4 clearIDValue{ -1.0f, 0.0f, 0.0f, 0.0f };
				m_FBO->ClearAttachment(1, clearIDValue);
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				m_ActiveScene->OnRender(*m_EditorCamera);
				m_PostSceneRenderDel.publish(m_FBO);
			}
		}
		EndFrameBuffer();
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

	void EditorBase::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RGBA16F, FrameBufferTextureFormat::Depth };
		fbSpec.Width = FRAMEBUFFER_WIDTH;
		fbSpec.Height = FRAMEBUFFER_HEIGHT;
		m_FBO = FrameBuffer::Create(fbSpec);
	}

	void EditorBase::BeginFrameBuffer()
	{
		m_FBO->Bind();
	}

	void EditorBase::EndFrameBuffer()
	{
		m_FBO->Unbind();
	}

}
