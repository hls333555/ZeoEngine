#include "ZEpch.h"
#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Core/RandomEngine.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	EngineLayer::EngineLayer(const std::string& name)
		: Layer(name)
	{
		RandomEngine::Init();
	}

	void EngineLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		Texture2D::s_DefaultBackgroundTexture = Texture2D::Create("resources/textures/DefaultBackground.png", true);
	}

}
