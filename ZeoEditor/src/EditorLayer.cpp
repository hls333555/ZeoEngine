#include "EditorLayer.h"

#include "Core/EditorManager.h"
#include "Engine/Core/AssetManager.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	EditorLayer::EditorLayer()
		: EngineLayer("Editor")
	{
	}

	void EditorLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		AssetManager::Get().Init();
		ThumbnailManager::Get().Init();
		AssetRegistry::Get().Init();

		EditorManager::Get().CreateEditor(EditorType::SceneEditor);
		
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

}
