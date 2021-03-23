#pragma once

#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	class MainDockspace : public DockspaceBase
	{
	public:
		using DockspaceBase::DockspaceBase;

		virtual void OnAttach() override;

		virtual PanelType GetViewportPanelType() const override { return PanelType::GameView; }

	private:
		virtual AssetType GetAssetType() const override { return AssetType::Scene; }

		void ClearSelectedEntity();

		virtual void Serialize(const std::string& filePath) override;
		virtual void Deserialize(const std::string& filePath) override;

		virtual void PreRenderDockspace() override;

		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}
