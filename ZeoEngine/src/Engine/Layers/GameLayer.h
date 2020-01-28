#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Core/TimerManager.h"
#include "Engine/Renderer/Texture.h"

struct ImFont;

namespace ZeoEngine {

	class Level;

	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

		inline Level* InternalGetLevel() { return m_Level; }
		inline TimerManager* InternalGetTimerManager() { return &m_TimerManager; }
		inline Texture2DLibrary* InternalGetTexture2DLibrary() { return &m_Texture2DLibrary; }
		inline ImFont* InternalGetFont() { return m_Font; }

		// TODO: LoadSharedTextures()
		void LoadSharedTextures();

	private:
		bool OnWindowResized(WindowResizeEvent& e);

		// TODO: CreateCamera() 
		void CreateCamera(uint32_t width, uint32_t height);

	private:
		Scope<OrthographicCamera> m_Camera;
		Level* m_Level;
		TimerManager m_TimerManager;
		Texture2DLibrary m_Texture2DLibrary;
		ImFont* m_Font;

	};

}
