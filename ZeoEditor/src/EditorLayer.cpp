#include "EditorLayer.h"

#include "Dockspaces/MainDockspace.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Debug/BenchmarkTimer.h"

namespace ZeoEngine {

#define SHOW_IMGUI_DEMO 0

	PIEState pieState;

	EditorLayer::EditorLayer()
		: EngineLayer("Editor")
	{
	}

	void EditorLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		MainDockspace* mainDockspace = new MainDockspace(EditorDockspaceType::Main_Editor, this, true, { 5.0f, 5.0f }, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
		PushDockspace(mainDockspace);

		// TODO:
		std::string cachePath = std::filesystem::current_path().string().append("/temp");
		// Create temp folder on demand
		std::filesystem::create_directory(cachePath);
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		EngineLayer::OnUpdate(dt);

		m_DockspaceManager.OnUpdate(dt);
	}

	void EditorLayer::OnImGuiRender()
	{
#if SHOW_IMGUI_DEMO
		static bool bShow = false;
		ImGui::ShowDemoWindow(&bShow);
#endif

		m_DockspaceManager.OnImGuiRender();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_DockspaceManager.OnEvent(event);
	}

	void EditorLayer::PushDockspace(EditorDockspace* dockspace)
	{
		m_DockspaceManager.PushDockspace(dockspace);
	}

	EditorDockspace* EditorLayer::GetDockspaceByType(EditorDockspaceType dockspaceType)
	{
		return m_DockspaceManager.GetDockspaceByName(ResolveEditorNameFromEnum(dockspaceType));
	}

	void EditorLayer::RebuildDockLayout(EditorDockspaceType dockspaceType)
	{
		std::string name = dockspaceType == EditorDockspaceType::NONE ? "" : ResolveEditorNameFromEnum(dockspaceType);
		m_DockspaceManager.RebuildDockLayout(name);
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Start PIE by pressing Alt+P
		if ((Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt)) && Input::IsKeyPressed(Key::P))
		{
			if (pieState == PIEState::None)
			{
				StartPIE();
			}
		}
		// Exit PIE by pressing Esc
		if (Input::IsKeyPressed(Key::Escape))
		{
			if (pieState != PIEState::None)
			{
				StopPIE();
			}
		}
		return false;
	}

	constexpr const char* PIETempFile = "temp/PIE.tmp";

	void EditorLayer::StartPIE()
	{
		ZE_CORE_TRACE("PIE started");
	}

	void EditorLayer::StopPIE()
	{
		ZE_CORE_TRACE("PIE stopped");
	}

	void EditorLayer::PausePIE()
	{
		ZE_CORE_TRACE("PIE paused");
	}

	void EditorLayer::ResumePIE()
	{
		ZE_CORE_TRACE("PIE resumed");
	}

}
