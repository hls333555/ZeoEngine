#include "Panels/AssetViewPanel.h"

#include "Worlds/AssetPreviewWorlds.h"
#include "Panels/AssetViews.h"

namespace ZeoEngine {

	AssetViewPanel::AssetViewPanel(std::string panelName)
		: ViewPanelBase(std::move(panelName))
	{
		m_PanelSpec.bDisableClose = true;
	}

	void AssetViewPanel::ProcessRender()
	{
		const auto startPos = ImGui::GetCursorPos();

		ViewPanelBase::ProcessRender();

		GetEditorWorld()->GetAssetView()->Draw();

		ImGui::SetCursorPos(startPos);
		GetEditorWorld()->GetAssetView()->DrawToolBar();
	}

}
