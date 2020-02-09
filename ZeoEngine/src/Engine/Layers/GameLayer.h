#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/OrthographicCameraController.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Core/TimerManager.h"

namespace ZeoEngine {

	class GameObject;

	class GameLayer : public Layer
	{
		friend class EditorLayer;
		friend class Level;

	public:
		GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

		OrthographicCamera* GetGameCamera() { return &m_GameCameraController->GetCamera(); }
		Texture2DLibrary* GetTexture2DLibrary() { return &m_Texture2DLibrary; }
		TimerManager* GetTimerManager() { return &m_TimerManager; }

	private:
		void LoadFont(const char* fontPath, const char* missingChars);

		// TODO: LoadSharedTextures()
		void LoadSharedTextures();

	private:
		Scope<OrthographicCameraController> m_GameCameraController;
		OrthographicCamera* m_ActiveCamera;
		Texture2DLibrary m_Texture2DLibrary;

		TimerManager m_TimerManager;

		std::vector<GameObject*> m_GameObjectsPendingDestroy;

		EditorLayer* m_EditorLayer;

	};

}
