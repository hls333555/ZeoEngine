#pragma once

#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Events/KeyEvent.h"

namespace ZeoEngine {

	class EditorDockspace;

	// TODO: PIEState
	enum class PIEState
	{
		None,
		Running,
		Paused
	};

	extern PIEState pieState;

	class EditorLayer : public EngineLayer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void StartPIE();
		void StopPIE();
		void PausePIE();
		void ResumePIE();

	};

}
