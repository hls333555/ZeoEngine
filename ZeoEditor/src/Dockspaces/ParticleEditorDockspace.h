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
		virtual AssetType GetAssetType() const override { return AssetType::ParticleTemplate; }

		void ReloadParticleTemplateData();

		virtual void Serialize(const std::string& filePath) override;
		virtual void Deserialize(const std::string& filePath) override;

		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}
