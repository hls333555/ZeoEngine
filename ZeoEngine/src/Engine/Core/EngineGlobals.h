#pragma once

#include "Engine/Core/Application.h"
#include "Engine/Layers/GameLayer.h"

namespace ZeoEngine {

#define Super __super

	template<typename T>
	static T* GetLevel()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? dynamic_cast<T*>(gameLayer->InternalGetLevel()) : nullptr;
	}

	static TimerManager* GetTimerManager()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->InternalGetTimerManager() : nullptr;
	}

	static Texture2DLibrary* GetTexture2DLibrary()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->InternalGetTexture2DLibrary() : nullptr;
	}

	static ImFont* GetFont()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->InternalGetFont() : nullptr;
	}

}
