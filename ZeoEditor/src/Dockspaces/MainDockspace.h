#pragma once

#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	class MainDockspace : public DockspaceBase
	{
	public:
		using DockspaceBase::DockspaceBase;

		virtual void OnAttach() override;

		virtual PanelType GetViewPanelType() const override { return PanelType::GameView; }

	private:
		virtual void PreRenderDockspace() override;

		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}
