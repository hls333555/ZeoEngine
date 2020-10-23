#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace ZeoEngine {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;

		virtual void OnDetach() override;

		void Begin();
		void End();

	private:
		void LoadDefaultFont();
		void SetDarkThemeColors();

	};

}
