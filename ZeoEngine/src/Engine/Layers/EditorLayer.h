#pragma once

#include "Engine/Core/Layer.h"

#include <glm/glm.hpp>

namespace ZeoEngine {

	class Level;

	class EditorLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;

	private:
		void ShowEditorDockspace();
		void ShowGameView(bool* bShow);
		void ShowLevelOutline(bool* bShow);
		void ShowConsole(bool* bShow);

		void ShowPreferences(bool* bShow);

	private:
		glm::vec2 m_LastGameViewSize;
		bool m_bResetLayout = false;

		Level* m_Level;
	};

}
