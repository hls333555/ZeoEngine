#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/OrthographicCameraController.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	class GameObject;
	class EditorLayer;

	class GameLayer : public Layer
	{
	public:
		GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

		const Scope<OrthographicCameraController>& GetGameCameraController() const { return m_GameCameraController; }
		OrthographicCamera* GetGameCamera() const { return m_GameCameraController ? &m_GameCameraController->GetCamera() : nullptr; }
		OrthographicCamera* GetActiveCamera() const { return m_ActiveCamera; }
		TimerManager* GetTimerManager() { return &m_TimerManager; }
		Texture2DLibrary* GetTexture2DLibrary() { return &m_Texture2DLibrary; }
		ParticleLibrary* GetParticleLibrary() { return &m_ParticleLibrary; }

		void AddGameObjectPendingDestroy(GameObject* object) { m_GameObjectsPendingDestroy.push_back(object); }

	private:
		void LoadFont(const char* fontPath, const char* missingChars);

	private:
		Scope<OrthographicCameraController> m_GameCameraController;
		OrthographicCamera* m_ActiveCamera;

		TimerManager m_CoreTimerManager, m_TimerManager;
		Texture2DLibrary m_Texture2DLibrary;
		ParticleLibrary m_ParticleLibrary;

		std::vector<GameObject*> m_GameObjectsPendingDestroy;
		float m_GarbageCollectionInterval;

		EditorLayer* m_Editor;

	};

}
