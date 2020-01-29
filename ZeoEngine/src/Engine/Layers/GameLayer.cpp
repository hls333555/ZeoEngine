#include "ZEpch.h"
#include "Engine/Layers/GameLayer.h"

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	GameLayer::GameLayer()
		: Layer("Game")
	{
		RandomEngine::Init();

		m_Level = CreateLevel();

		const auto& window = Application::Get().GetWindow();
		CreateCamera(window.GetWidth(), window.GetHeight());

	}

	GameLayer::~GameLayer()
	{
		delete m_Level;
	}

	void GameLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		// TODO: Load font
		ImGuiIO& io = ImGui::GetIO();
		m_Font = io.Fonts->AddFontFromFileTTF("assets/fonts/wqy-microhei.ttc", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

		LoadSharedTextures();

		m_Level->Init();

	}

	void GameLayer::OnDetach()
	{
		ZE_PROFILE_FUNCTION();

	}

	void GameLayer::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNCTION();

		m_TimerManager.OnUpdate(dt);
		m_Level->OnUpdate(dt);

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
				m_Level->OnRender();
				Renderer2D::EndScene();
			}
		}
		Renderer2D::EndRenderingToTexture();
	}

	void GameLayer::OnImGuiRender()
	{
		ZE_PROFILE_FUNCTION();

		m_Level->OnImGuiRender();
	}

	void GameLayer::OnEvent(Event& event)
	{
		ZE_PROFILE_FUNCTION();

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
		m_Level->SetLevelBounds({ left, right, bottom, top });
	}

	void GameLayer::LoadSharedTextures()
	{
		m_Texture2DLibrary.Load("assets/textures/Explosion_2x4.png");

	}

}
