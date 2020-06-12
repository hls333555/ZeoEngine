#include "ZEpch.h"
#include "Engine/GameFramework/EngineLayer.h"

#include <imgui.h>

#include "Engine/Core/RandomEngine.h"

namespace ZeoEngine {

	EngineLayer::EngineLayer(const std::string& name)
		: Layer(name)
	{
		RandomEngine::Init();
	}

	void EngineLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		// TODO: Move it to config file
		// NOTE: Add missing Chinese characters here!
		static const char* missingChars = u8"¼­äÖÄâÖ¡";
		LoadFont("assets/fonts/wqy-microhei.ttc", missingChars);

		//// A very basic GameObject based garbage collection system
		//m_CoreTimerManager.SetTimer(m_GarbageCollectionInterval, nullptr, [&]() {
		//	BenchmarkTimer bt(false);
		//	level.RemoveGameObjects();
		//	for (auto* object : m_GameObjectsPendingDestroy)
		//	{
		//		delete object;
		//	}
		//	m_GameObjectsPendingDestroy.clear();
		//	ZE_CORE_INFO("Garbage collection took {0}s.", bt.GetDuration());
		//	}, 0);

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
