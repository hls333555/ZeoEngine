#include "EditorLayer.h"

#include "Core/EditorManager.h"
#include "Core/AssetManager.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	PIEState pieState;

	EditorLayer::EditorLayer()
		: EngineLayer("Editor")
	{
	}

	void EditorLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		EditorManager::Get().CreateEditor(EditorType::MainEditor);
		AssetManager::Get().Init();
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		EngineLayer::OnUpdate(dt);

		// TODO: Check
		Renderer2D::ResetStats();
		EditorManager::Get().OnUpdate(dt);
	}

	void EditorLayer::OnImGuiRender()
	{
#if ZE_SHOW_IMGUI_DEMO
		static bool bShow = false;
		ImGui::ShowDemoWindow(&bShow);
#endif

		EditorManager::Get().OnImGuiRender();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		EditorManager::Get().OnEvent(event);
	}

	// TODO:
	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		//// Start PIE by pressing Alt+P
		//if ((Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt)) && Input::IsKeyPressed(Key::P))
		//{
		//	if (pieState == PIEState::None)
		//	{
		//		StartPIE();
		//	}
		//}
		//// Exit PIE by pressing Esc
		//if (Input::IsKeyPressed(Key::Escape))
		//{
		//	if (pieState != PIEState::None)
		//	{
		//		StopPIE();
		//	}
		//}
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
