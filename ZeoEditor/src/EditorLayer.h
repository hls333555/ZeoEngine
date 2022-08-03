#pragma once

#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorLayer : public EngineLayer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	};

}
