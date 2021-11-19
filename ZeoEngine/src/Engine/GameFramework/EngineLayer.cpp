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


	}

}
