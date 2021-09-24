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
		NewAsset(false);
		m_EditorUIRenderer = CreateEditorUIRenderer();
		m_EditorUIRenderer->OnAttach();
	}

	void EditorBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		m_EditorUIRenderer->OnUpdate(dt);

		m_Scene->OnUpdate(dt);

		BeginFrameBuffer();
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
				RenderCommand::SetClearColor(clearColor);
				RenderCommand::Clear();
				// Clear entity ID attachment to -1
				m_FBO->ClearAttachment(1, -1);
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				m_Scene->OnRender(*m_EditorCamera);
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
			m_Scene->OnEvent(e);
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

	void EditorBase::NewAsset(bool bIsFromLoad)
	{
		m_PreSceneCreateDel.publish(bIsFromLoad);
		m_Scene = CreateScene();
		m_PostSceneCreateDel.publish(bIsFromLoad);
	}

	void EditorBase::LoadAsset()
	{
		auto openAssetPanel = m_EditorUIRenderer->GetPanel<OpenAssetPanel>(OPEN_ASSET);
		if (!openAssetPanel)
		{
			openAssetPanel = m_EditorUIRenderer->CreatePanel<OpenAssetPanel>(OPEN_ASSET, SceneAsset::TypeId());
		}
		else
		{
			openAssetPanel->Open();
		}
	}

	void EditorBase::LoadAsset(const std::string& path)
	{
		BenchmarkTimer timer;

		NewAsset(true);
		LoadAssetImpl(path);
		m_Scene->PostLoad();
		m_PostSceneLoadDel.publish();

		ZE_CORE_WARN("Loading \"{0}\" took {1} ms", path, timer.ElapsedMillis());
	}

	void EditorBase::SaveAsset()
	{
		const std::string assetPath = GetAssetPath();
		if (assetPath.empty())
		{
			SaveAssetAs();
		}
		else
		{
			SaveAsset(assetPath);
		}
	}

	void EditorBase::SaveAsset(const std::string& path)
	{
		BenchmarkTimer timer;

		SaveAssetImpl(path);

		ZE_CORE_WARN("Saving {0} took {1} ms", path, timer.ElapsedMillis());
	}

	void EditorBase::SaveAssetAs()
	{
		auto saveAssetPanel = m_EditorUIRenderer->GetPanel<SaveAssetPanel>(SAVE_ASSET);
		if (!saveAssetPanel)
		{
			saveAssetPanel = m_EditorUIRenderer->CreatePanel<SaveAssetPanel>(SAVE_ASSET, SceneAsset::TypeId());
		}
		else
		{
			saveAssetPanel->Open();
		}
	}

	void EditorBase::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth };
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
