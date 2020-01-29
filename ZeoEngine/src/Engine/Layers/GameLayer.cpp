#include "ZEpch.h"
#include "Engine/Layers/GameLayer.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Layers/EditorLayer.h"

namespace ZeoEngine {

	GameLayer::GameLayer()
		: Layer("Game")
	{
		RandomEngine::Init();

		const auto& window = Application::Get().GetWindow();
		CreateCamera(window.GetWidth(), window.GetHeight());

		editor = Application::Get().FindLayerByName<EditorLayer>("Editor");
	}

	void GameLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		LoadSharedTextures();

		Level::Get().Init();

	}

	void GameLayer::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNCTION();

		if (editor->GetPIEState() == PIEState::Running)
		{
			Level::Get().OnUpdate(dt);
		}

		Renderer2D::BeginRenderingToTexture();
		// Render
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				RenderCommand::Clear();
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				Renderer2D::BeginScene(*m_Camera);
				Level::Get().OnRender();
				Renderer2D::EndScene();
			}
		}
		Renderer2D::EndRenderingToTexture();
	}

	void GameLayer::OnImGuiRender()
	{
		ZE_PROFILE_FUNCTION();

		Level::Get().OnImGuiRender();
	}

	void GameLayer::CreateCamera(uint32_t width, uint32_t height)
	{
		float aspectRatio = (float)width / (float)height;
		float zoomLevel = 4.0f;
		float left = -zoomLevel * aspectRatio;
		float right = zoomLevel * aspectRatio;
		float bottom = -zoomLevel;
		float top = zoomLevel;
		m_Camera = CreateScope<OrthographicCamera>(left, right, bottom, top);
		Level::Get().SetLevelBounds({ left, right, bottom, top });
	}

	void GameLayer::LoadSharedTextures()
	{
		m_Texture2DLibrary.Load("assets/textures/Explosion_2x4.png");

	}

}
