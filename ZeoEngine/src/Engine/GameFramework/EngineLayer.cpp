#include "ZEpch.h"
#include "Engine/GameFramework/EngineLayer.h"

#include <imgui.h>

#include "Engine/Core/RandomEngine.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	EngineLayer::EngineLayer(const std::string& name)
		: Layer(name)
		, m_GarbageCollectionInterval(30.0f)
	{
		RandomEngine::Init();
	}

	void EngineLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		Texture2DLibrary::Get().Load("../ZeoEditor/assets/textures/Checkerboard_Alpha.png");

		// TODO: Move it to config file
		// NOTE: Add missing Chinese characters here!
		static const char* missingChars = u8"¼­äÖÄâÖ¡";
		LoadFont("../ZeoEditor/assets/fonts/wqy-microhei.ttc", missingChars);

		Level& level = Level::Get();

		// A very basic GameObject based garbage collection system
		m_CoreTimerManager.SetTimer(m_GarbageCollectionInterval, nullptr, [&]() {
			BenchmarkTimer bt(false);
			level.RemoveGameObjects();
			for (auto* object : m_GameObjectsPendingDestroy)
			{
				delete object;
			}
			m_GameObjectsPendingDestroy.clear();
			ZE_CORE_INFO("Garbage collection took {0}s.", bt.GetDuration());
		}, 0);

	}

	void EngineLayer::OnDetach()
	{
		for (auto* object : m_GameObjectsPendingDestroy)
		{
			delete object;
		}
	}

	void EngineLayer::OnUpdate(DeltaTime dt)
	{
		m_CoreTimerManager.OnUpdate(dt);
	}

	void EngineLayer::LoadFont(const char* fontPath, const char* missingChars)
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
