#pragma once

#include "Engine/GameFramework/EngineLayer.h"

#include "Engine/Events/KeyEvent.h"
#include "Dockspaces/EditorDockspace.h"

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

		void PushDockspace(EditorDockspace* dockspace);
		EditorDockspace* GetDockspaceByType(EditorDockspaceType dockspaceType);
		/** If dockspaceType is NONE, all dockspaces will be reset layout. */
		void RebuildDockLayout(EditorDockspaceType dockspaceType = EditorDockspaceType::NONE);

	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void StartPIE();
		void StopPIE();
		void PausePIE();
		void ResumePIE();

	private:
		DockspaceManager m_DockspaceManager;

	};

}
