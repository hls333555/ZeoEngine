#include "ZEpch.h"
#include "GameLayer.h"

#include "Engine/Core/Application.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	GameLayer::GameLayer()
		: EngineLayer("Game")
		, m_GarbageCollectionInterval(30.0f)
	{
		const auto& window = Application::Get().GetWindow();
		m_CameraController = CreateScope<OrthographicCameraController>(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight()));
		m_CameraController->SetZoomLevel(3.0f);

	}

	void GameLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		Level& level = Level::Get();
		level.Init(nullptr);
		// TODO: Move it to config file
		// Load default game level
		level.LoadLevelFromFile("assets/test.zlevel");
	}

	void GameLayer::OnDetach()
	{
		for (auto* object : m_GameObjectsPendingDestroy)
		{
			delete object;
		}
	}

	void GameLayer::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNCTION();

		m_CoreTimerManager.OnUpdate(dt);

		Level::Get().OnUpdate(dt);

		{
			Renderer2D::ResetStats();
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				RenderCommand::Clear();
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				Renderer2D::BeginScene(m_CameraController->GetCamera());
				Level::Get().OnRender();
				Renderer2D::EndScene();
			}
		}
	}

}
