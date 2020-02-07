#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/OrthographicCameraController.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class EditorLayer;

	class GameLayer : public Layer
	{
		friend class EditorLayer;

	public:
		GameLayer();

		virtual void OnAttach() override;

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

		OrthographicCamera* GetGameCamera() { return &m_GameCameraController->GetCamera(); }
		Texture2DLibrary* GetTexture2DLibrary() { return &m_Texture2DLibrary; }

	private:
		void LoadFont(const char* fontPath, const char* missingChars);

		// TODO: LoadSharedTextures()
		void LoadSharedTextures();

	private:
		Scope<OrthographicCameraController> m_GameCameraController;
		OrthographicCamera* m_ActiveCamera;
		Texture2DLibrary m_Texture2DLibrary;

		EditorLayer* m_EditorLayer;

	};

}
