#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	class EditorLayer;

	class GameLayer : public Layer
	{
	public:
		GameLayer();

		virtual void OnAttach() override;

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

		inline Texture2DLibrary* InternalGetTexture2DLibrary() { return &m_Texture2DLibrary; }

		// TODO: CreateCamera() 
		void CreateCamera(uint32_t width, uint32_t height);

		// TODO: LoadSharedTextures()
		void LoadSharedTextures();

	private:
		Scope<OrthographicCamera> m_Camera;
		Texture2DLibrary m_Texture2DLibrary;

		EditorLayer* editor;

	};

}
