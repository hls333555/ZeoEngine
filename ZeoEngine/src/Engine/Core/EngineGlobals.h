#pragma once

#include "Engine/GameFramework/Level.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/Core/Application.h"
#include "Engine/Layers/GameLayer.h"
#include "Engine/Renderer/OrthographicCamera.h"

namespace ZeoEngine {

	static OrthographicCamera* GetActiveGameCamera()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->GetGameCamera() : nullptr;
	}

	static TimerManager* GetTimerManager()
	{
		return Level::Get().GetTimerManager();
	}

	static Texture2DLibrary* GetTexture2DLibrary()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->GetTexture2DLibrary() : nullptr;
	}

}
