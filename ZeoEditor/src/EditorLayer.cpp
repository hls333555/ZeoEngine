#include "EditorLayer.h"

#include "Core/WindowManager.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/CoreMacros.h"

namespace ZeoEngine {

	PIEState pieState;

	EditorLayer::EditorLayer()
		: EngineLayer("Editor")
	{
	}

	void EditorLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		DockspaceManager::Get().CreateDockspace(DockspaceType::MainEditor);
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		EngineLayer::OnUpdate(dt);

		DockspaceManager::Get().OnUpdate(dt);
	}

	void EditorLayer::OnImGuiRender()
	{
#if SHOW_IMGUI_DEMO
		static bool bShow = false;
		ImGui::ShowDemoWindow(&bShow);
#endif

		DockspaceManager::Get().OnImGuiRender();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		DockspaceManager::Get().OnEvent(event);
	}

	// TODO:
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
