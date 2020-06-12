#pragma once

#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/OrthographicCameraController.h"
#include "Engine/Core/TimerManager.h"

namespace ZeoEngine {

	class GameObject;
	class EditorLayer;

	class GameLayer : public EngineLayer
	{
	public:
		GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(DeltaTime dt) override;

		virtual OrthographicCamera* GetGameCamera() override { return &m_CameraController->GetCamera(); }

		void AddGameObjectPendingDestroy(GameObject* object) { m_GameObjectsPendingDestroy.insert(object); }

	private:
		Scope<OrthographicCameraController> m_CameraController;

		TimerManager m_CoreTimerManager{ "Core" };

		std::set<GameObject*> m_GameObjectsPendingDestroy;
		float m_GarbageCollectionInterval;

	};

}
