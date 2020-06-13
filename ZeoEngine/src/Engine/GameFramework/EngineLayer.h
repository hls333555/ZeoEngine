#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	class EngineLayer : public Layer
	{
	public:
		EngineLayer(const std::string& name = "Layer");

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(DeltaTime dt) override;

		virtual OrthographicCamera* GetGameCamera() = 0;

		Texture2DLibrary* GetTexture2DLibrary() { return &m_Texture2DLibrary; }
		ParticleLibrary* GetParticleLibrary() { return &m_ParticleLibrary; }

		void AddGameObjectPendingDestroy(GameObject* object) { m_GameObjectsPendingDestroy.insert(object); }

	private:
		void LoadFont(const char* fontPath, const char* missingChars);

	private:
		TimerManager m_CoreTimerManager{ "Core" };
		Texture2DLibrary m_Texture2DLibrary;
		ParticleLibrary m_ParticleLibrary;

		float m_GarbageCollectionInterval;
		std::set<GameObject*> m_GameObjectsPendingDestroy;

	};

}
