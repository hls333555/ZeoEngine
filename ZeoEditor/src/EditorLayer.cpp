#include "EditorLayer.h"

#include "Core/Editor.h"
#include "Core/EditorTypes.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Profile/Profiler.h"
#include "Engine/Renderer/Renderer.h"

namespace ZeoEngine {

	EditorLayer::EditorLayer()
		: Layer("Editor")
	{
	}

	void EditorLayer::OnAttach()
	{
		// TODO:
		AssetManager::Get().Init();
		ThumbnailManager::Get().Init();
		AssetRegistry::Get().Register();

		m_Editor = CreateScope<Editor>();
		m_Editor->OnAttach();
	}

	void EditorLayer::OnDetach()
	{
		m_Editor->OnDetach();

		AssetManager::Get().Shutdown();
		ThumbnailManager::Get().Shutdown();
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		Renderer::ResetStats();

		m_Editor->OnUpdate(dt);
	}

	void EditorLayer::OnImGuiRender()
	{
		ZE_PROFILE_FUNC();

		m_Editor->OnImGuiRender();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_Editor->OnEvent(event);
	}

}
