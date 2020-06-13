#pragma once

#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Renderer/OrthographicCameraController.h"

namespace ZeoEngine {

	class GameLayer : public EngineLayer
	{
	public:
		GameLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

		virtual OrthographicCamera* GetGameCamera() override { return &m_CameraController->GetCamera(); }

	private:
		Scope<OrthographicCameraController> m_CameraController;

	};

}
