#include "ZEpch.h"
#include "Engine/Layers/GameLayer.h"

#include <imgui.h>

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
		// TODO: Add an interface for user to create custom game camera
		m_GameCameraController = CreateScope<OrthographicCameraController>((float)window.GetWidth() / (float)window.GetHeight());
		m_GameCameraController->SetZoomLevel(3.0f);

		m_EditorLayer = Application::Get().FindLayerByName<EditorLayer>("Editor");
		ZE_CORE_ASSERT_INFO(m_EditorLayer, "GameLayer: m_EditorLayer is null!");
	}

	void GameLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		// NOTE: Add missing Chinese characters here!
		static const char* missingChars = u8"¼­äÖ";
		LoadFont("assets/fonts/wqy-microhei.ttc", missingChars);

		LoadSharedTextures();

		Level::Get().Init();

	}

	void GameLayer::OnUpdate(DeltaTime dt)
	{
		ZE_PROFILE_FUNCTION();

		if (m_EditorLayer->m_PIEState == PIEState::None)
		{
			// Setting editor camera
			m_ActiveCamera = &m_EditorLayer->m_EditorCameraController->GetCamera();
		}
		else
		{
			// Setting game camera
			m_ActiveCamera = &m_GameCameraController->GetCamera();
			if (m_EditorLayer->m_PIEState == PIEState::Running)
			{
				Level::Get().OnUpdate(dt);
			}
		}

		Renderer2D::BeginRenderingToTexture();
		{
			{
				ZE_PROFILE_SCOPE("Renderer Prep");

				RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				RenderCommand::Clear();
			}
			{
				ZE_PROFILE_SCOPE("Renderer Draw");

				Renderer2D::BeginScene(*m_ActiveCamera);
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

	void GameLayer::LoadFont(const char* fontPath, const char* missingChars)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFontGlyphRangesBuilder builder;
		// Load 2500 common Chinese characters
		builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
		// Add needed missing Chinese characters
		builder.AddText(missingChars);
		ImVector<ImWchar> OutRanges;
		builder.BuildRanges(&OutRanges);
		io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, NULL, &OutRanges[0]);
		unsigned char* outPixels = nullptr;
		int outWidth, outHeight, outBytesPerPixel;
		io.Fonts->GetTexDataAsAlpha8(&outPixels, &outWidth, &outHeight, &outBytesPerPixel);
	}

	// TODO: Load all textures inside assets/textures
	void GameLayer::LoadSharedTextures()
	{
		m_Texture2DLibrary.Load("assets/textures/Explosion_2x4.png");

	}

}
