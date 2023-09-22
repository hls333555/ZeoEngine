#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"

namespace ZeoEngine {

	class ImGuiLayer : public Layer
	{
		friend class Application;

	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool bBlock) { m_bBlockEvents = bBlock; }

	private:
		void LoadDefaultFont();
		void SetDarkThemeColors();

	private:
		bool m_bBlockEvents = false;
		bool m_bShowImGuiStats = false;
	};

}
