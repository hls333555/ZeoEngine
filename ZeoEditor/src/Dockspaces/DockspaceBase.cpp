#include "Dockspaces/DockspaceBase.h"

#include <imgui_internal.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Debug/Instrumentor.h"
#include "Scenes/MainEditorScene.h"
#include "Scenes/ParticleEditorScene.h"
#include "Engine/Utils/PlatformUtils.h"

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

namespace ZeoEngine {

	DockspaceBase::DockspaceBase(const DockspaceSpec& spec)
		: m_DockspaceSpec(spec)
	{
	}

	void DockspaceBase::OnAttach()
	{
		CreateScene();
		CreateFrameBuffer();
	}

	void DockspaceBase::OnUpdate(DeltaTime dt)
	{
		if (!m_bShow) return;

		m_PanelManager.OnUpdate(dt);

		m_Scene->OnUpdate(dt);

		// TODO: Move these to other place
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

	void DockspaceBase::OnImGuiRender()
	{
		if (!m_bShow) return;

		// Render dockspace
		PreRenderDockspace();
		RenderDockspace();
		// Render panels
		m_PanelManager.OnImGuiRender();
	}

	void DockspaceBase::OnEvent(Event& e)
	{
		if (!m_bShow) return;

		m_MenuManager.OnEvent(e);
		m_PanelManager.OnEvent(e);

		if (!m_bBlockSceneEvents)
		{
			m_Scene->OnEvent(e);
		}
	}

	void DockspaceBase::PreRenderDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImVec2 centerPos{ mainViewport->Pos.x + mainViewport->Size.x / 2.0f, mainViewport->Pos.y + mainViewport->Size.y / 2.0f };
		ImGui::SetNextWindowPos(centerPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(m_DockspaceSpec.InitialSize.Data, m_DockspaceSpec.InitialSize.Condition);
	}

	void DockspaceBase::RenderDockspace()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		const char* dockspaceName = GetDockspaceName(m_DockspaceSpec.Type);
		ImGui::Begin(dockspaceName, &m_bShow, m_DockspaceSpec.WindowFlags);
		ImGui::PopStyleVar();

		m_bIsDockspaceFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		m_bIsDockspaceHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		// Render menus (non-main-menus must be rendered winthin window context)
		m_MenuManager.OnImGuiRender(m_DockspaceSpec.Type == DockspaceType::MainEditor);

		ImGuiID dockspaceID = ImGui::GetID(dockspaceName);
		if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr || m_bShouldRebuildDockLayout)
		{
			m_bShouldRebuildDockLayout = false;

			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(dockspaceID);
			// Add empty node
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
			// Main node should cover entire window
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());
			// Build dock layout
			BuildDockWindows(dockspaceID);
			ImGui::DockBuilderFinish(dockspaceID);
		}

		// Should be put at last
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::End();
	}

	void DockspaceBase::FocusContextEntity()
	{
		m_EditorCamera->StartFocusEntity(m_ContextEntity);
	}

	EditorMenu& DockspaceBase::CreateMenu(const std::string& menuName)
	{
		return m_MenuManager.CreateMenu(menuName, this);
	}

	void DockspaceBase::CreatePanel(PanelType panelType)
	{
		m_PanelManager.CreatePanel(panelType, this);
	}

	PanelBase* DockspaceBase::TogglePanel(PanelType panelType, bool bOpen)
	{
		return m_PanelManager.TogglePanel(panelType, this, bOpen);
	}

	void DockspaceBase::CreateNewScene(bool bIsFromOpenScene)
	{
		m_PreSceneCreateDel.publish(bIsFromOpenScene);
		CreateScene();
		m_PostSceneCreateDel.publish(bIsFromOpenScene);
	}

	void DockspaceBase::OpenScene()
	{
		auto filePath = FileDialogs::OpenFile(GetAssetType());
		if (!filePath) return;

		CreateNewScene(true);
		// Save scene path
		m_Scene->SetPath(*filePath);
		Deserialize(*filePath);
		m_Scene->OnDeserialized();
	}

	void DockspaceBase::SaveSceneAs()
	{
		auto filePath = FileDialogs::SaveFile(GetAssetType());
		if (!filePath) return;

		// Save scene path
		m_Scene->SetPath(*filePath);
		Serialize(*filePath);
	}

	void DockspaceBase::SaveScene()
	{
		const std::string scenePath = m_Scene->GetPath();
		if (scenePath.empty())
		{
			SaveSceneAs();
		}
		else
		{
			Serialize(scenePath);
		}
	}

	void DockspaceBase::CreateScene()
	{
		switch (m_DockspaceSpec.Type)
		{
			case DockspaceType::MainEditor:		m_Scene = CreateRef<MainEditorScene>(); return;
			case DockspaceType::ParticleEditor:	m_Scene = CreateRef<ParticleEditorScene>(); return;
		}

		ZE_CORE_ASSERT(false);
	}

	void DockspaceBase::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth };
		fbSpec.Width = FRAMEBUFFER_WIDTH;
		fbSpec.Height = FRAMEBUFFER_HEIGHT;
		m_FBO = FrameBuffer::Create(fbSpec);
	}

	void DockspaceBase::BeginFrameBuffer()
	{
		m_FBO->Bind();
	}

	void DockspaceBase::EndFrameBuffer()
	{
		m_FBO->Unbind();
	}

}
