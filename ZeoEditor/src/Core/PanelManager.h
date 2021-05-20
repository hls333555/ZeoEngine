#pragma once

#include "Core/EditorTypes.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class PanelBase;
	class EditorBase;

	class PanelManager
	{
		friend class DockspaceBase;

	private:
		PanelManager() = default;
		~PanelManager();

	public:
		PanelBase* CreatePanel(PanelType type, EditorBase* owningEditor);
		PanelBase* OpenPanel(PanelType type, EditorBase* owningEditor);
		PanelBase* GetPanel(PanelType type);

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

	private:
		std::unordered_map<PanelType, PanelBase*> m_Panels;
	};

}
