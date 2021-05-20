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

				RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
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

	void EditorBase::CreateNewScene(bool bIsFromOpenScene)
	{
		m_PreSceneCreateDel.publish(bIsFromOpenScene);
		CreateScene();
		m_PostSceneCreateDel.publish(bIsFromOpenScene);
	}

	void EditorBase::OpenScene()
	{
		auto filePath = FileDialogs::OpenFile(GetAssetType());
		if (!filePath) return;

		OpenScene(*filePath);
	}

	void EditorBase::OpenScene(const std::string& path)
	{
		CreateNewScene(true);
		// Save scene path
		m_Scene->SetPath(path);
		Deserialize(path);
		m_Scene->OnDeserialized();
	}

	void EditorBase::SaveSceneAs()
	{
		auto filePath = FileDialogs::SaveFile(GetAssetType());
		if (!filePath) return;

		// Save scene path
		m_Scene->SetPath(*filePath);
		Serialize(*filePath);
	}

	void EditorBase::SaveScene()
	{
		const std::string& scenePath = m_Scene->GetPath();
		if (scenePath.empty())
		{
			SaveSceneAs();
		}
		else
		{
			Serialize(scenePath);
		}
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
