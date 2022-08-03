#include "EditorLayer.h"

#include "Core/EditorManager.h"
#include "Editors/LevelEditor.h"
#include "Core/EditorTypes.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Renderer/Renderer.h"

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

		const Ref<LevelEditor> levelEditor = EditorManager::Get().CreateEditor<LevelEditor>(LEVEL_EDITOR);
		
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		EngineLayer::OnUpdate(dt);

		Renderer::ResetStats();
		EditorManager::Get().OnUpdate(dt);
		AssetRegistry::Get().OnUpdate(dt);
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
