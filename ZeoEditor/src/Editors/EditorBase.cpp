#include "Editors/EditorBase.h"

#include "Dockspaces/MainEditorDockspace.h"
#include "Dockspaces/ParticleEditorDockspace.h"
#include "Scenes/MainEditorScene.h"
#include "Scenes/ParticleEditorScene.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Debug/Instrumentor.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Debug/BenchmarkTimer.h"

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

namespace ZeoEngine {

	EditorBase::EditorBase(EditorType type)
		: m_EditorType(type)
	{
	}

	EditorBase::~EditorBase()
	{
		delete m_Dockspace;
	}

	void EditorBase::OnAttach()
	{
		CreateScene();
		CreateFrameBuffer();
		CreateDockspace();
	}

	void EditorBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		m_Dockspace->OnUpdate(dt);

		m_Scene->OnUpdate(dt);

		BeginFrameBuffer();
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				glm::vec4 clearColor = { 0.25f, 0.25f, 0.25f, 1.0f };
				if (m_bPendingClearColorTransparent)
				{
					clearColor.a = 0.0f;
				}
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

		m_Dockspace->OnImGuiRender();
	}

	void EditorBase::OnEvent(Event& e)
	{
		if (!m_bShow) return;

		m_Dockspace->OnEvent(e);

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
		CreateScene();
		PostSceneCreate(bIsFromLoad);
		m_PostSceneCreateDel.publish(bIsFromLoad);
	}

	void EditorBase::LoadAsset()
	{
		auto filePath = FileDialogs::OpenFile(GetAssetTypeId());
		if (!filePath) return;

		LoadAsset(*filePath);
	}

	void EditorBase::LoadAsset(const std::string& path)
	{
		BenchmarkTimer timer;

		NewAsset(true);
		LoadAssetImpl(path);
		m_Scene->PostLoad();
		m_PostSceneLoadDel.publish();

		ZE_CORE_WARN("Loading {0} took {1} ms", path, timer.ElapsedMillis());
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
		auto filePath = FileDialogs::SaveFile(GetAssetTypeId());
		if (!filePath) return;

		SaveAsset(*filePath);
	}

	void EditorBase::CreateDockspace()
	{
		switch (m_EditorType)
		{
			case EditorType::MainEditor:		m_Dockspace = new MainEditorDockspace(this); break;
			case EditorType::ParticleEditor:	m_Dockspace = new ParticleEditorDockspace(this); break;
			default:							ZE_CORE_ASSERT(false, "Unknown EditorType!"); return;
		}

		m_Dockspace->OnAttach();
	}

	void EditorBase::CreateScene()
	{
		switch (m_EditorType)
		{
			case EditorType::MainEditor:		m_Scene = CreateRef<MainEditorScene>(); return;
			case EditorType::ParticleEditor:	m_Scene = CreateRef<ParticleEditorScene>(); return;
			default:							ZE_CORE_ASSERT(false, "Unknown EditorType!"); return;
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
