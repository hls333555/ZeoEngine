#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class Editor;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		Ref<Editor> m_Editor;
	};

}
