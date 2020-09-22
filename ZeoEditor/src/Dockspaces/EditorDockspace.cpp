#include "Dockspaces/EditorDockspace.h"

#include <imgui_internal.h>

#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Debug/Instrumentor.h"

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

namespace ZeoEngine {

	EditorDockspace::EditorDockspace(const std::string& dockspaceName, float dockspaceRounding, float dockspaceBorderSize, ImVec2 dockspacePadding, ImGuiWindowFlags dockspaceWindowFlags, ImVec2 dockspacePos, ImVec2 dockspaceSize)
		: m_DockspaceName(dockspaceName)
		, m_DockspaceRounding(dockspaceRounding), m_DockspaceBorderSize(dockspaceBorderSize), m_DockspacePadding(dockspacePadding)
		, m_DockspaceWindowFlags(dockspaceWindowFlags)
		, m_DockspacePos(dockspacePos), m_DockspaceSize(dockspaceSize)
	{
	}

	void EditorDockspace::OnAttach()
	{
		CreateScene();
		CreateFrameBuffer();
	}

	void EditorDockspace::OnUpdate(DeltaTime dt)
	{
		m_Scene->OnUpdate(dt);

		BeginFrameBuffer();
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
				RenderCommand::Clear();
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				m_Scene->OnRender();
			}
		}
		EndFrameBuffer();
	}

	void EditorDockspace::OnImGuiRender()
	{
		// Dockspace
		RenderDockspace();
		// Menus
		m_MenuManager.OnImGuiRender(m_bIsMainDockspace);
		// Panels
		m_PanelManager.OnImGuiRender();
	}

	void EditorDockspace::RenderDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		// TODO: Needs bFirstOnly? 
		ImGui::SetNextWindowPos(m_bIsMainDockspace ? mainViewport->Pos : m_DockspacePos);
		ImGui::SetNextWindowSize(m_bIsMainDockspace ? mainViewport->Size : m_DockspaceSize);
		//ImGui::SetNextWindowViewport(mainViewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_DockspaceRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_DockspaceBorderSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_DockspacePadding);

		ImGui::Begin(m_DockspaceName.c_str(), &m_bShow, m_DockspaceWindowFlags);
		ImGui::PopStyleVar(3);

		// TODO: Needs separate from window name?
		ImGuiID dockspaceID = ImGui::GetID(m_DockspaceName.c_str());
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

	void EditorDockspace::PushMenu(EditorMenu* menu)
	{
		m_MenuManager.PushMenu(menu);
	}

	void EditorDockspace::PushPanel(EditorPanel* panel)
	{
		m_PanelManager.PushPanel(panel);
	}

	ZeoEngine::EditorPanel* EditorDockspace::GetPanelByName(const std::string& panelName)
	{
		return m_PanelManager.GetPanelByName(panelName);
	}

	void EditorDockspace::CreateScene()
	{
		m_Scene = CreateRef<Scene>();
	}

	void EditorDockspace::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Width = FRAMEBUFFER_WIDTH;
		fbSpec.Height = FRAMEBUFFER_HEIGHT;
		m_FBO = FrameBuffer::Create(fbSpec);
	}

	void EditorDockspace::BeginFrameBuffer()
	{
		m_FBO->Bind();
	}

	void EditorDockspace::EndFrameBuffer()
	{
		m_FBO->Unbind();
	}

	DockspaceManager::~DockspaceManager()
	{
		for (auto* dockspace : m_Dockspaces)
		{
			dockspace->OnDetach();
			delete dockspace;
		}
	}

	void DockspaceManager::OnUpdate(DeltaTime dt)
	{
		for (auto* dockspace : m_Dockspaces)
		{
			dockspace->OnUpdate(dt);
		}
	}

	void DockspaceManager::OnImGuiRender()
	{
		for (auto* dockspace : m_Dockspaces)
		{
			dockspace->OnImGuiRender();
		}
	}

	void DockspaceManager::PushDockspace(EditorDockspace* dockspace)
	{
		m_Dockspaces.emplace_back(dockspace);
		dockspace->OnAttach();
	}

	void DockspaceManager::RebuildDockLayout(const std::string& dockspaceName)
	{
		bool bShouldRebuildAll = dockspaceName.empty();
		for (auto* dockspace : m_Dockspaces)
		{
			if (bShouldRebuildAll || dockspace->GetDockspaceName() == dockspaceName)
			{
				dockspace->m_bShouldRebuildDockLayout = true;
				if (!bShouldRebuildAll) break;
			}
		}
	}

}
