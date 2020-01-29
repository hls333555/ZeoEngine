#pragma once

#include "Engine/GameFramework/Level.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/Core/Application.h"
#include "Engine/Layers/GameLayer.h"

namespace ZeoEngine {

#define Super __super

	static TimerManager* GetTimerManager()
	{
		return Level::Get().InternalGetTimerManager();
	}

	static Texture2DLibrary* GetTexture2DLibrary()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->InternalGetTexture2DLibrary() : nullptr;
	}

}
