#pragma once

#include "Engine/Core/Application.h"
#include "Engine/Layers/GameLayer.h"

namespace ZeoEngine {

	static OrthographicCamera* GetActiveGameCamera()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->GetGameCamera() : nullptr;
	}

	static TimerManager* GetTimerManager()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->GetGameTimerManager() : nullptr;
	}

	static Texture2DLibrary* GetTexture2DLibrary()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->GetTexture2DLibrary() : nullptr;
	}

	static ParticleLibrary* GetParticleLibrary()
	{
		GameLayer* gameLayer = Application::Get().FindLayerByName<GameLayer>("Game");
		return gameLayer ? gameLayer->GetParticleLibrary() : nullptr;
	}

}
