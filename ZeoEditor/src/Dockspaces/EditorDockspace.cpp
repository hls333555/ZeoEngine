#include "Dockspaces/EditorDockspace.h"

#include <imgui_internal.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Debug/Instrumentor.h"
#include "EditorLayer.h"
#include "Panels/SceneViewportPanel.h"
#include "Scenes/MainEditorScene.h"
#include "Scenes/ParticleEditorScene.h"

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

namespace ZeoEngine {

	EditorDockspace::EditorDockspace(EditorDockspaceType dockspaceType, EditorLayer* context, bool bDefaultShow, const glm::vec2& dockspacePadding, ImGuiWindowFlags dockspaceWindowFlags, ImVec2Data initialSize, ImVec2Data initialPos)
		: m_DockspaceType(dockspaceType), m_EditorContext(context), m_bShow(bDefaultShow)
		, m_DockspacePadding(dockspacePadding)
		, m_DockspaceWindowFlags(dockspaceWindowFlags)
		, m_InitialSize(initialSize), m_InitialPos(initialPos)
	{
	}

	void EditorDockspace::OnAttach()
	{
		CreateScene();
		CreateFrameBuffers();
	}

	void EditorDockspace::OnUpdate(DeltaTime dt)
	{
		m_PanelManager.OnUpdate(dt);

		m_Scene->OnUpdate(dt);

		BeginFrameBuffer();
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
				RenderCommand::Clear();
				// TODO: This fixes ReadPixels() != -1 when nothing selected
				BeginFrameBuffer();
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				m_Scene->OnRender(*m_EditorCamera);
				PostSceneRender();
			}
		}
		EndFrameBuffer();
	}

	void EditorDockspace::OnImGuiRender()
	{
		// Render dockspace
		RenderDockspace();
		// Render panels
		m_PanelManager.OnImGuiRender();
	}

	void EditorDockspace::OnEvent(Event& e)
	{
		m_MenuManager.OnEvent(e);
		m_PanelManager.OnEvent(e);

		if (!m_bBlockEvents)
		{
			m_Scene->OnEvent(e);
		}
	}

	void EditorDockspace::RenderDockspace()
	{
		if (!m_bShow) return;

		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		bool bIsMainDockspace = m_DockspaceType == EditorDockspaceType::Main_Editor;
		if (bIsMainDockspace)
		{
			ImGui::SetNextWindowViewport(mainViewport->ID);
			ImGui::SetNextWindowPos(mainViewport->Pos);
		}
		else if (m_InitialPos == ImVec2Data::DefaultPos)
		{
			ImVec2 CenterPos{ mainViewport->Size.x / 2.0f, mainViewport->Size.y / 2.0f };
			ImGui::SetNextWindowPos(CenterPos, m_InitialPos.Condition, ImVec2(0.5f, 0.5f));
		}
		else
		{
			ImGui::SetNextWindowPos(m_InitialPos.Data, m_InitialPos.Condition);
		}
		ImGui::SetNextWindowSize(bIsMainDockspace ? mainViewport->Size : static_cast<ImVec2>(m_InitialSize.Data), bIsMainDockspace ? 0 : m_InitialSize.Condition);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, bIsMainDockspace ? 0.0f : ImGui::GetStyle().WindowRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, bIsMainDockspace ? 0.0f : ImGui::GetStyle().WindowBorderSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_DockspacePadding);

		ImGui::Begin(GetDockspaceName().c_str(), &m_bShow, m_DockspaceWindowFlags);
		ImGui::PopStyleVar(3);

		m_bIsDockspaceFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		m_bIsDockspaceHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		// Render menus - non-main-menus must be rendered winthin window context
		m_MenuManager.OnImGuiRender(bIsMainDockspace);

		// TODO: Needs separate from window name?
		ImGuiID dockspaceID = ImGui::GetID(GetDockspaceName().c_str());
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

	void EditorDockspace::PushDockspace(EditorDockspace* dockspace)
	{
		m_EditorContext->PushDockspace(dockspace);
	}

	void EditorDockspace::PushMenu(EditorMenu* menu)
	{
		menu->SetContext(this);
		m_MenuManager.PushMenu(menu);
	}

	void EditorDockspace::PushPanel(EditorPanel* panel)
	{
		m_PanelManager.PushPanel(panel);
	}

	EditorDockspace* EditorDockspace::OpenEditor(EditorDockspaceType dockspaceType)
	{
		auto* editor = m_EditorContext->GetDockspaceByType(dockspaceType);
		*editor->GetShowPtr() = true;
		return editor;
	}

	void EditorDockspace::CreateNewScene(bool bIsFromOpenScene)
	{
		CreateScene();
		m_OnSceneCreateDel.publish(bIsFromOpenScene);
	}

	void EditorDockspace::OpenScene()
	{
		auto filePath = FileDialogs::OpenFile(GetAssetType());
		if (!filePath) return;

		CreateNewScene(true);
		// Save scene path
		m_Scene->SetPath(*filePath);
		Deserialize(*filePath);
		m_Scene->OnDeserialized();
	}

	void EditorDockspace::SaveSceneAs()
	{
		auto filePath = FileDialogs::SaveFile(GetAssetType());
		if (!filePath) return;

		// Save scene path
		m_Scene->SetPath(*filePath);
		Serialize(*filePath);
	}

	void EditorDockspace::SaveScene()
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

	void EditorDockspace::CreateScene()
	{
		switch (m_DockspaceType)
		{
		case EditorDockspaceType::Main_Editor:
			m_Scene = CreateRef<MainEditorScene>();
			break;
		case EditorDockspaceType::Particle_Editor:
			m_Scene = CreateRef<ParticleEditorScene>();
			break;
		default:
			ZE_CORE_ASSERT(false);
			break;
		}
	}

	void EditorDockspace::CreateFrameBuffers()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::Depth };
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
		for (auto& [name, dockspace] : m_Dockspaces)
		{
			dockspace->OnDetach();
			delete dockspace;
		}
	}

	void DockspaceManager::OnUpdate(DeltaTime dt)
	{
		Renderer2D::ResetStats();

		for (auto& [name, dockspace] : m_Dockspaces)
		{
			// Do not update scene if this dockspace is invisible
			if (dockspace->m_bShow)
			{
				dockspace->OnUpdate(dt);
			}
		}
	}

	void DockspaceManager::OnImGuiRender()
	{
		for (auto& [name, dockspace] : m_Dockspaces)
		{
			dockspace->OnImGuiRender();
		}
	}

	void DockspaceManager::OnEvent(Event& e)
	{
		for (auto& [name, dockspace] : m_Dockspaces)
		{
			dockspace->OnEvent(e);
		}
	}

	void DockspaceManager::PushDockspace(EditorDockspace* dockspace)
	{
		m_Dockspaces.emplace(dockspace->GetDockspaceName(), dockspace);
		dockspace->OnAttach();
	}

	EditorDockspace* DockspaceManager::GetDockspaceByName(const std::string& dockspaceName)
	{
		auto result = m_Dockspaces.find(dockspaceName);
		return result == m_Dockspaces.end() ? nullptr : result->second;
	}

	void DockspaceManager::RebuildDockLayout(const std::string& dockspaceName)
	{
		bool bShouldRebuildAll = dockspaceName.empty();
		for (auto& [name, dockspace] : m_Dockspaces)
		{
			if (bShouldRebuildAll || dockspace->GetDockspaceName() == dockspaceName)
			{
				dockspace->m_bShouldRebuildDockLayout = true;
				if (!bShouldRebuildAll) break;
			}
		}
	}

}