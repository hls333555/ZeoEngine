#include "Panels/EditorPanel.h"

#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	Vec2Data Vec2Data::DefaultPos;
	Vec2Data Vec2Data::CenterPos{ { -1.0f, -1.0f } };
	Vec2Data Vec2Data::DefaultSize{ { 400.0f, 500.0f } };

	EditorPanel::EditorPanel(const std::string& panelName, bool bDefaultShow, ImGuiWindowFlags panelWindowFlags, const Vec2Data& initialSize, const Vec2Data& initialPos)
		: m_PanelName(panelName), m_bShow(bDefaultShow), m_PanelWindowFlags(panelWindowFlags)
		, m_InitialSize(initialSize), m_InitialPos(initialPos)
	{
	}

	void EditorPanel::OnImGuiRender()
	{
		if (!m_bShow) return;

		if (m_InitialPos.Data.x < 0.0f)
		{
			ImGuiViewport* mainViewport = ImGui::GetMainViewport();
			ImVec2 CenterPos{ mainViewport->Size.x / 2.0f, mainViewport->Size.y / 2.0f };
			ImGui::SetNextWindowPos(CenterPos, m_InitialPos.Condition, ImVec2(0.5f, 0.5f));
		}
		else
		{
			ImGui::SetNextWindowPos(m_InitialPos.Data, m_InitialPos.Condition);
		}
		ImGui::SetNextWindowSize(m_InitialSize.Data, m_InitialSize.Condition);

		if (ImGui::Begin(m_PanelName.c_str(), &m_bShow, m_PanelWindowFlags))
		{
			m_bIsHovering = ImGui::IsWindowHovered();
		}
		ImGui::End();
	}

	ScenePanel::ScenePanel(const std::string& panelName, EditorDockspace* context, bool bDefaultShow, ImGuiWindowFlags panelWindowFlags, const Vec2Data& initialSize, const Vec2Data& initialPos)
		: EditorPanel(panelName, bDefaultShow, panelWindowFlags, initialSize, initialPos)
		, m_Context(context)
	{
	}

	const Ref<Scene>& ScenePanel::GetScene() const
	{
		return m_Context->GetScene();
	}

	const Ref<FrameBuffer>& ScenePanel::GetFrameBuffer() const
	{
		return m_Context->GetFrameBuffer();
	}

	PanelManager::~PanelManager()
	{
		for (auto& [name, panel] : m_Panels)
		{
			panel->OnDetach();
			delete panel;
		}
	}

	void PanelManager::OnImGuiRender()
	{
		for (auto& [name, panel] : m_Panels)
		{
			panel->OnImGuiRender();
		}
	}

	void PanelManager::PushPanel(EditorPanel* panel)
	{
		m_Panels.emplace(panel->GetPanelName(), panel);
		panel->OnAttach();
	}

	EditorPanel* PanelManager::GetPanelByName(const std::string& panelName)
	{
		auto result = m_Panels.find(panelName);
		return result == m_Panels.end() ? nullptr : result->second;
	}

}
