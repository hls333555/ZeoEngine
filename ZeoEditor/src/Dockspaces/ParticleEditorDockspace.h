#pragma once

#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	class ParticleEditorDockspace : public DockspaceBase
	{
	public:
		using DockspaceBase::DockspaceBase;

		virtual void OnAttach() override;

		virtual PanelType GetViewportPanelType() const override { return PanelType::ParticleView; }

	private:
		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}
