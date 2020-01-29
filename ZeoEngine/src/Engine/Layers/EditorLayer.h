#pragma once

#include "Engine/Core/Layer.h"

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	enum class PIEState
	{
		None,
		Running,
		Paused
	};

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

		inline PIEState GetPIEState() const { return m_PIEState; }

	private:
		void LoadTextures();

		void ShowEditorDockspace();
		void ShowGameView(bool* bShow);
		void ShowLevelOutline(bool* bShow);
		void ShowObjectProperty(bool* bShow);
		void ShowObjectBrowser(bool* bShow);
		void ShowConsole(bool* bShow);

		void ShowParticleEditor(bool* bShow);

		void ShowPreferences(bool* bShow);
		void ShowAbout(bool* bShow);

		void SetNextWindowDefaultPosition();

		void StartPIE();
		void StopPIE();
		void PausePIE();
		void ResumePIE();

	private:
		Ref<Texture2D> m_PlayTexture, m_PauseTexture, m_StopTexture, m_CurrentPlayTexture;

		glm::vec2 m_LastGameViewSize;
		bool m_bResetLayout = false;

		PIEState m_PIEState;

	};

}
