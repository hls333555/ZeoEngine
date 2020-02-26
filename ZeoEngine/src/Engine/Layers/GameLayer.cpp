#include "ZEpch.h"
#include "Engine/Layers/GameLayer.h"

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Layers/EditorLayer.h"
#include "Engine/Debug/BenchmarkTimer.h"

namespace ZeoEngine {

	GameLayer::GameLayer()
		: Layer("Game")
		, m_GarbageCollectionInterval(30.0f)
	{
		RandomEngine::Init();

		const auto& window = Application::Get().GetWindow();
		// TODO: Add an interface for user to create custom game camera
		m_GameCameraController = CreateScope<OrthographicCameraController>((float)window.GetWidth() / (float)window.GetHeight());
		m_GameCameraController->SetZoomLevel(3.0f);

		m_Editor = Application::Get().FindLayerByName<EditorLayer>("Editor");
		ZE_CORE_ASSERT_INFO(m_Editor, "GameLayer::m_Editor is null!");
	}

	void GameLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		// NOTE: Add missing Chinese characters here!
		static const char* missingChars = u8"º≠‰÷ƒ‚÷°";
		LoadFont("assets/fonts/wqy-microhei.ttc", missingChars);

		Level::Get().Init();

		// A very basic GameObject based garbage collection system
		m_CoreTimerManager.SetTimer(m_GarbageCollectionInterval, [&]() {
			BenchmarkTimer bt(false);
			for (uint32_t i = 0; i < m_GameObjectsPendingDestroy.size(); ++i)
			{
				delete m_GameObjectsPendingDestroy[i];
			}
			m_GameObjectsPendingDestroy.clear();
			ZE_CORE_INFO("Garbage collection took {0}s.", bt.GetDuration());
		}, 0);

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

		if (g_PIEState == PIEState::None)
		{
			// Setting editor camera
			m_ActiveCamera = m_Editor->GetGameViewCamera();
		}
		else
		{
			// Setting game camera
			m_ActiveCamera = &m_GameCameraController->GetCamera();
			if (g_PIEState == PIEState::Running)
			{
				m_TimerManager.OnUpdate(dt);
				Level::Get().OnUpdate(dt);
			}
		}

		Renderer2D::BeginRenderingToTexture(0);
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
		Renderer2D::EndRenderingToTexture(0);
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

}
