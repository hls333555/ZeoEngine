#pragma once

#include "Engine/Core/Application.h"
#include "Engine/GameFramework/EngineLayer.h"
#include "Engine/GameFramework/Level.h"

namespace ZeoEngine {

	static OrthographicCamera* GetActiveGameCamera()
	{
		EngineLayer* engineLayer = Application::Get().GetEngineLayer();
		return engineLayer ? engineLayer->GetGameCamera() : nullptr;
	}

	static TimerManager* GetTimerManager()
	{
		return Level::Get().GetGameTimerManager();
	}

}
