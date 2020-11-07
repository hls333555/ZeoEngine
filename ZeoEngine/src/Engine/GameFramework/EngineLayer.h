#pragma once

#include "Engine/Core/Layer.h"

#include <set>

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Core/TimerManager.h"

namespace ZeoEngine {

	class EngineLayer : public Layer
	{
	public:
		EngineLayer(const std::string& name = "Layer");

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(DeltaTime dt) override;

		virtual OrthographicCamera* GetGameCamera() { return nullptr; }

		void AddGameObjectPendingDestroy(GameObject* object) { m_GameObjectsPendingDestroy.insert(object); }

	private:
		TimerManager m_CoreTimerManager{ "Core" };

		float m_GarbageCollectionInterval;
		std::set<GameObject*> m_GameObjectsPendingDestroy;

	};

}
